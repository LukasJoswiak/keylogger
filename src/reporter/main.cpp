#include <iostream>

#include "reporter.hpp"
#include "../utilities/cli.hpp"

int main(int argc, char** argv) {
  try {
    ArgParser p(argv, argv + argc);
    auto begin = p.parse_date("--begin");
    auto end = p.parse_date("--end");
    if (begin) {
      std::cout << begin->time_since_epoch().count() << std::endl;
    }
    if (end) {
      std::cout << end->time_since_epoch().count() << std::endl;
    }

    auto input = p.parse_string("--input");

    Reporter r(input.value(), begin, end);

    auto counts = r.GetCounts();
    for (const auto& [key, value] : counts) {
      std::cout << key << ": " << value << std::endl;
    }
  } catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << std::endl;
  }
}
