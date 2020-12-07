#pragma once

#include <ApplicationServices/ApplicationServices.h>

#include <fstream>

class Recorder {
 public:
  Recorder(const std::string& out_path);

  void Handle(CGEventType type, CGEventRef event);

 private:
  std::ofstream ostrm_;
};
