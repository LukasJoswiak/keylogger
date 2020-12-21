#pragma once

#include <fstream>
#include <map>
#include <optional>
#include <string>

class Reporter {
 public:
  Reporter(
      const std::string& in_path,
      std::optional<std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>> begin,
      std::optional<std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>> end);

  // Processes the input file and returns a map of Unicode characters to their
  // frequency.
  std::map<uint16_t, uint64_t> GetCounts();

 private:
  // Minimum file version that can be parsed.
  static constexpr uint64_t kMinReadVersion = 1;
  // Maximum file version that can be parsed.
  static constexpr uint64_t kMaxReadVersion = 1;
  static_assert(kMinReadVersion <= kMaxReadVersion);

  // Seeks to the correct begin location based on user parameters.
  void SeekToBeginning();

  std::ifstream istrm_;
  uint64_t version_;
  uint32_t vendor_id_;
  uint32_t product_id_;

  std::optional<std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>> begin_;
  std::optional<std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>> end_;
};
