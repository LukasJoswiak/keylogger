#include "reporter.hpp"

#include <iostream>

Reporter::Reporter(const std::string& in_path) {
  istrm_ = std::ifstream(in_path, std::ios::binary);

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
  istrm_.seekg(sizeof(version_) + sizeof(vendor_id_) + sizeof(product_id_));

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

    if (counts.find(scancode) == counts.end()) {
      counts[scancode] = 0;
    }
    ++counts[scancode];
  }

  return counts;
}
