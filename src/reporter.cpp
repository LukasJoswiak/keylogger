#include "reporter.hpp"

Reporter::Reporter(const std::string& in_path) {
  istrm_ = std::ifstream(in_path, std::ios::binary);

  istrm_.read(reinterpret_cast<char*>(&version_), sizeof(version_));
  if (version_ < kMinReadVersion || version_ > kMaxReadVersion) {
    throw std::runtime_error("cannot read input file: invalid version");
  }
}

std::map<uint16_t, uint64_t> Reporter::GetCounts() {
  istrm_.seekg(sizeof(version_));

  std::map<uint16_t, uint64_t> counts;
  
  while (!istrm_.eof()) {
    int64_t timestamp;
    uint16_t keycode;
    istrm_.read(reinterpret_cast<char*>(&timestamp), sizeof(timestamp));
    istrm_.read(reinterpret_cast<char*>(&keycode), sizeof(keycode));
    if (counts.find(keycode) == counts.end()) {
      counts[keycode] = 0;
    }
    ++counts[keycode];
  }

  return counts;
}
