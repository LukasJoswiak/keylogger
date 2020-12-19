#include "recorder.hpp"

#include <Carbon/Carbon.h>
#include <IOKit/IOKitLib.h>

#include <cassert>
#include <chrono>
#include <iostream>

Recorder::Recorder(const std::string& out_path, bool record_virtual) : ostrm_(out_path, std::ios::binary | std::ios::app), record_virtual_(record_virtual) {
  uint16_t record_virtual_data = 0;
  if (record_virtual) {
    record_virtual_data = 1;
  }

  // If there is an existing data file, make sure its version matches the
  // current write version. If the versions don't match, stop execution to
  // avoid corrupting the file by writing data in an incompatible format.
  if (ostrm_.tellp() > 0) {
    std::ifstream istrm = std::ifstream(out_path, std::ios::binary | std::ios::in);
    auto version = decltype(kWriteVersion){0};
    istrm.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (version != kWriteVersion) {
      throw std::runtime_error("output file has existing data encoded at a different version");
    }

    // Verify virtual flag in output file matches passed setting.
    auto file_virtual = decltype(record_virtual_data){0};
    istrm.read(reinterpret_cast<char*>(&file_virtual), sizeof(file_virtual));
    if (file_virtual != record_virtual_data) {
      throw std::runtime_error("output file has different virtual flag");
    }

    istrm.seekg(6, std::ios::cur);
  } else {
    // Write the version, in little-endian, to the new file.
    ostrm_.write(reinterpret_cast<const char*>(&kWriteVersion), sizeof(kWriteVersion));

    // Write the data type, in little-endian. This represents whether the
    // recorded keystrokes are virtual or not.
    ostrm_.write(reinterpret_cast<const char*>(&record_virtual_data), sizeof(uint16_t));

    // Fill up remaining 48-bits with zero.
    uint64_t zero = 0;
    ostrm_.write(reinterpret_cast<const char*>(&zero), 6);
  }
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
  // the screen.
  uint32_t scancode = IOHIDElementGetUsage(element);

  // My impression is that for keyboards, this value will be 1 if the key has
  // been pressed, and 0 if the key has been released. I'll go based on this
  // assumption for now.
  long pressed = IOHIDValueGetIntegerValue(value);

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
  std::cout << "scancode: " << scancode
            << ", pressed: " << pressed
            << ", vendor ID: " << vendor_id
            << ", product ID: " << product_id
            << ", timestamp: " << timestamp
            << std::endl;

  // TODO: Serialize information and save it to an output file named with the
  // vendor and product IDs.
}

void Recorder::HIDKeyboardCallback(void* context, IOReturn result, void* sender, IOHIDValueRef value) {
  Recorder* r = static_cast<Recorder*>(context);
  r->Handle(result, sender, value);
}
