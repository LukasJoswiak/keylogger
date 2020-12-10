#pragma once

#include <ApplicationServices/ApplicationServices.h>

#include <fstream>
#include <string>

class Recorder {
 public:
  Recorder(const std::string& out_path);

  void Handle(CGEventType type, CGEventRef event);

 private:
  // Given a virtual keycode and a keyboard event, convert the keycode to the
  // Unicode character it will produce and return the Unicode character as it's
  // 16-bit representation.
  //
  // Note that this function truncates Unicode characters with encodings larger
  // than 2^16 - 1. See the implementation for more info.
  uint16_t KeycodeToUnicode(uint16_t virtual_keycode, CGEventRef event);

  static constexpr uint64_t kWriteVersion = 0x1;

  std::ofstream ostrm_;
};
