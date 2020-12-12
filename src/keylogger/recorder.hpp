#pragma once

#include <ApplicationServices/ApplicationServices.h>

#include <fstream>
#include <string>

class Recorder {
 public:
  Recorder(const std::string& out_path);

  void Handle(CGEventType type, CGEventRef event);

 private:
  static constexpr uint64_t kWriteVersion = 0x1;

  std::ofstream ostrm_;
};