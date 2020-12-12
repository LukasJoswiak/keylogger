#include <iostream>

#include "reporter.hpp"

int main(int argc, char** argv) {
  try {
    Reporter r(argv[1]);

    auto counts = r.GetCounts();
    for (const auto& [key, value] : counts) {
      std::cout << key << ": " << value << std::endl;
    }
  } catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << std::endl;
  }
}
