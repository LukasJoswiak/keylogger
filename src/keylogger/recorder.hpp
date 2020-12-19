#pragma once

#include <ApplicationServices/ApplicationServices.h>
#include <IOKit/hid/IOHIDValue.h>

#include <fstream>
#include <string>
#include <unordered_map>

class Recorder {
 public:
  Recorder(const std::string& out_dir);

  static void HIDKeyboardCallback(void* context, IOReturn result, void* sender, IOHIDValueRef value);

  void Handle(IOReturn result, void* sender, IOHIDValueRef value);

 private:
  static constexpr uint64_t kWriteVersion = 0x1;

  // Given the vendor ID and product ID of a USB HID device, returns an output
  // stream object which the caller should write associated device data to.
  std::ofstream& GetStream(uint32_t vendor_id, uint32_t product_id);

  // Directory containing saved data about keystrokes for each device.
  std::string out_dir_;
  std::unordered_map<uint64_t, std::ofstream> streams_;
};
