#pragma once

#include <ApplicationServices/ApplicationServices.h>

#include <unordered_map>

#include "recorder.hpp"

class Keylogger {
 public:
  Keylogger(const std::string& out_path, bool record_virtual = false);

  void Run();
  void Stop();

 private:
  // Creates a CFMutableDictionary out of the given key-value pairs. Caller is
  // responsible for freeing returned dictionary.
  CFMutableDictionaryRef CreateMatchingCriteria(std::unordered_map<const char*, int> criteria);

  Recorder recorder_;
};
