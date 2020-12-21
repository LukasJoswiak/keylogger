#include "reporter.hpp"

#include <iostream>

Reporter::Reporter(const std::string& in_path,
    std::optional<std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>> begin,
    std::optional<std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>> end)
      : istrm_(in_path, std::ios::binary), begin_(begin), end_(end) {
  istrm_.read(reinterpret_cast<char*>(&version_), sizeof(version_));
  if (version_ < kMinReadVersion || version_ > kMaxReadVersion) {
    throw std::runtime_error("cannot read input file: invalid version");
  }

  uint64_t device;
  istrm_.read(reinterpret_cast<char*>(&device), sizeof(device));
  vendor_id_ = static_cast<uint32_t>(device >> (8 * sizeof(product_id_)) & 0xffff);
  product_id_ = static_cast<uint32_t>(device & 0xffff);

  std::cout << "Vendor ID: " << vendor_id_ << std::endl;
  std::cout << "Product ID: " << product_id_ << std::endl;
}

std::map<uint16_t, uint64_t> Reporter::GetCounts() {
  SeekToBeginning();

  std::map<uint16_t, uint64_t> counts;
  
  while (!istrm_.eof()) {
    int64_t timestamp;
    uint8_t scancode;
    bool pressed;
    istrm_.read(reinterpret_cast<char*>(&timestamp), sizeof(timestamp));
    istrm_.read(reinterpret_cast<char*>(&scancode), sizeof(scancode));
    istrm_.read(reinterpret_cast<char*>(&pressed), sizeof(pressed));

    if (!pressed) {
      continue;
    }

    std::chrono::microseconds ms(timestamp);
    std::chrono::time_point<std::chrono::system_clock> tp(ms);
    if (end_.has_value() && tp > end_.value()) {
      break;
    }

    if (counts.find(scancode) == counts.end()) {
      counts[scancode] = 0;
    }
    ++counts[scancode];
  }

  return counts;
}

void Reporter::SeekToBeginning() {
  istrm_.seekg(sizeof(version_) + sizeof(vendor_id_) + sizeof(product_id_));

  if (!begin_.has_value()) {
    return;
  }

  while (!istrm_.eof()) {
    int64_t timestamp;
    uint8_t scancode;
    bool pressed;
    istrm_.read(reinterpret_cast<char*>(&timestamp), sizeof(timestamp));
    istrm_.read(reinterpret_cast<char*>(&scancode), sizeof(scancode));
    istrm_.read(reinterpret_cast<char*>(&pressed), sizeof(pressed));

    std::chrono::microseconds ms(timestamp);
    std::chrono::time_point<std::chrono::system_clock> tp(ms);
    if (tp >= begin_.value()) {
      // Seek back one unit of data to account for reading a timestamp that is
      // >= to the timestamp being searched for.
      istrm_.seekg(-1 * (sizeof(timestamp) + sizeof(scancode) + sizeof(pressed)), std::ios::cur);
      break;
    }
  }
}
