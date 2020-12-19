#pragma once

#include <ApplicationServices/ApplicationServices.h>
#include <IOKit/hid/IOHIDValue.h>

#include <fstream>
#include <string>

class Recorder {
 public:
  Recorder(const std::string& out_path, bool record_virtual);

  void Handle(CGEventType type, CGEventRef event);
  void Handle(IOReturn result, void* sender, IOHIDValueRef value);

  static void HIDKeyboardCallback(void* context, IOReturn result, void* sender, IOHIDValueRef value);

 private:
  static constexpr uint64_t kWriteVersion = 0x1;

  std::ofstream ostrm_;
  bool record_virtual_;
};
