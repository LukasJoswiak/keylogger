#include "recorder.hpp"

#include <IOKit/hid/IOHIDDevice.h>
#include <IOKit/hid/IOHIDElement.h>
#include <IOKit/hid/IOHIDUsageTables.h>

#include <cassert>
#include <chrono>
#include <iostream>

Recorder::Recorder(const std::string& out_dir) : out_dir_(out_dir) {}

void Recorder::HIDKeyboardCallback(void* context, IOReturn result, void* sender, IOHIDValueRef value) {
  Recorder* r = static_cast<Recorder*>(context);
  r->Handle(result, sender, value);
}

void Recorder::Handle(IOReturn result, void* sender, IOHIDValueRef value) {
  // IOHIDValueGetTimeStamp(value) can be used to get the timestamp of the HID
  // event, but the timestamp is represented in OS absolute time, or the number
  // of processor ticks since boot (while awake). We want a datetime, and since
  // exact nanosecond precision isn't *too* important here, we'll just use the
  // system clock to get the current timestamp.
  int64_t timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();

  IOHIDElementRef element = IOHIDValueGetElement(value);
  if (IOHIDElementGetUsagePage(element) != kHIDPage_KeyboardOrKeypad) {
    return;
  }

  // The scan code is a number representing which key on the keyboard was
  // pressed. All scan codes are defined in the keyboard/keypad page (0x07) of
  // the USB HID specification. Note that the scan code is a representation of
  // the physical key pressed, not the character the computer will output on
  // the screen. The largest possible value returned should be 0xe7 as defined
  // by the constraints set in keylogger.cpp with
  // IOHIDManagerSetInputValueMatching.
  uint8_t scancode = static_cast<uint8_t>(IOHIDElementGetUsage(element));

  // My impression is that for keyboards, this value will be 1 if the key has
  // been pressed, and 0 if the key has been released. I'll go based on this
  // assumption for now.
  bool pressed = IOHIDValueGetIntegerValue(value) > 0;

  // Read the vendor and product IDs of the device sending the HID report. It
  // is necessary to perform this step on every HID report received because it
  // is possible for multiple devices to be used simultaneously.
  int32_t vendor_id = 0;
  int32_t product_id = 0;
  {
    IOHIDDeviceRef device = static_cast<IOHIDDeviceRef>(sender);

    CFNumberRef vendor_id_property = static_cast<CFNumberRef>(IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey)));
    assert(vendor_id_property != nullptr);
    CFNumberGetValue(vendor_id_property, kCFNumberSInt32Type, &vendor_id);

    CFNumberRef product_id_property = static_cast<CFNumberRef>(IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey)));
    assert(product_id_property != nullptr);
    CFNumberGetValue(product_id_property, kCFNumberSInt32Type, &product_id);
  }

  // Print some info out for now.
  std::cout << "scancode: " << (uint16_t) scancode
            << ", pressed: " << pressed
            << ", vendor ID: " << vendor_id
            << ", product ID: " << product_id
            << ", timestamp: " << timestamp
            << std::endl;

  //
  // Serialization format (v1):
  //
  // header:
  // +-------------------+ +-------------------+
  // |      version      | |     device ID     |
  // +-------------------+ +-------------------+
  //        64 bits               64 bits
  //
  // body (repeating):
  // +-------------------+ +---------+ +-------+
  // |     timestamp     | | keycode | | state | . . .
  // +-------------------+ +---------+ +-------+
  //        64 bits           8 bits     1 bit
  //
  // version: 64-bit unsigned integer representing the serialization version
  // device ID: 64-bit unsigned integer representing a unique identifier for
  //            the device. 32 higher order bits are the vendor ID, and the 32
  //            lower order bits are the product ID of the HID device
  //
  // timestamp: 64-bit signed integer, microseconds since epoch, representing
  //            the timestamp of the event
  // scancode: 8-bit unsigned integer, usage ID of the selected key according
  //           to the USB HID 0x07 usage page
  // state: 1-bit value representing whether the key has been pressed or
  //        released. Will be set to 1 if the state of the key is pressed, and
  //        0 if the state of the key is released
  //
  std::ofstream& ostrm = GetStream(vendor_id, product_id);
  ostrm.write(reinterpret_cast<const char*>(&timestamp), sizeof(timestamp));
  ostrm.write(reinterpret_cast<const char*>(&scancode), sizeof(scancode));
  ostrm.write(reinterpret_cast<const char*>(&pressed), sizeof(pressed));
}

std::ofstream& Recorder::GetStream(uint32_t vendor_id, uint32_t product_id) {
  // Need a fast way to combine the vendor ID and product ID into a uniquely
  // identifiable value. I guess the correct thing to do here would be to
  // implement a pairing function. Instead, I'll just use a 64-bit unsigned
  // integer where the higher 32 bits represent the vendor ID, and the lower 32
  // bits represent the product ID. This value will be used to identify the
  // input device.
  uint64_t device_id = (static_cast<uint64_t>(vendor_id) << (8 * sizeof(product_id))) | product_id;

  if (streams_.find(device_id) == streams_.end()) {
    const std::string out_path = out_dir_ + "/" + std::to_string(device_id);

    // Lazily instantiate output stream the first time it's needed.
    streams_.emplace(device_id, std::ofstream(out_path, std::ios::binary | std::ios::app));
    std::ofstream& ostrm = streams_[device_id];
    if (ostrm.fail()) {
      throw std::runtime_error("failed to open output file for writing");
    }

    if (ostrm.tellp() > 0) {
      // If there is an existing data file, make sure its version matches the
      // current write version. If the versions don't match, stop execution to
      // avoid corrupting the file by writing data in an incompatible format.
      std::ifstream istrm = std::ifstream(out_path, std::ios::binary | std::ios::in);
      assert(istrm.good());
      auto version = decltype(kWriteVersion){0};
      istrm.read(reinterpret_cast<char*>(&version), sizeof(version));
      if (version != kWriteVersion) {
        throw std::runtime_error("output file has existing data encoded at a different version");
      }

      // Verify device ID in output file matches the current device ID.
      uint64_t file_device_id;
      istrm.read(reinterpret_cast<char*>(&file_device_id), sizeof(file_device_id));
      if (file_device_id != device_id) {
        throw std::runtime_error("output file name doesn't match internal metadata for device ID");
      }
    } else {
      // If no output file exists for the device yet, write some initial
      // metadata to the new file.

      // Write the version, in little-endian, to the new file.
      ostrm.write(reinterpret_cast<const char*>(&kWriteVersion), sizeof(kWriteVersion));

      // Write the device ID, in little-endian.
      ostrm.write(reinterpret_cast<const char*>(&device_id), sizeof(device_id));
    }
  }

  return streams_[device_id];
}
