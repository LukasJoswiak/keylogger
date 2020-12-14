#include "cli.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

ArgParser::ArgParser(char** begin, char** end) : begin_(begin), end_(end) {}

bool ArgParser::exists(const std::string& option) {
  return std::find(begin_, end_, option) != end_;
}

std::optional<std::string> ArgParser::parse_string(const std::string& option) {
  char** iterator = std::find(begin_, end_, option);
  if (iterator != end_ && ++iterator != end_) {
    return *iterator;
  }
  return std::nullopt;
}

std::optional<std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>> ArgParser::parse_date(const std::string& option) {
  auto string_date = parse_string(option);
  if (string_date) {
    std::tm tm;
    std::stringstream ss(string_date.value());
    ss >> std::get_time(&tm,  "%Y-%m-%dT%H:%M:%S");
    if (ss.fail()) {
      throw std::runtime_error("failed to parse value for " + option);
    }
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
  }
  return std::nullopt;
}
