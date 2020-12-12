#pragma once

#include <fstream>
#include <map>
#include <string>

class Reporter {
 public:
  Reporter(const std::string& in_path);

  // Processes the input file and returns a map of Unicode characters to their
  // frequency.
  std::map<uint16_t, uint64_t> GetCounts();

 private:
  // Minimum file version that can be parsed.
  static constexpr uint64_t kMinReadVersion = 1;
  // Maximum file version that can be parsed.
  static constexpr uint64_t kMaxReadVersion = 1;
  static_assert(kMinReadVersion <= kMaxReadVersion);

  std::ifstream istrm_;
  uint64_t version_;
};
