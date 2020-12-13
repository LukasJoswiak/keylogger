// Contains utilities to assist with CLI tasks such as argument parsing.

#include <chrono>
#include <optional>
#include <string>

class ArgParser {
 public:
  ArgParser(char** begin, char** end);

  std::optional<std::string> parse_string(const std::string& option);
  // Parses a date in the ISO 8601 extended format.
  std::optional<std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>> parse_date(const std::string& option);

 private:
  char** begin_;
  char** end_;
};
