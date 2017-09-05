#include "http.hpp"
#include <string>
#include <unordered_map>
#include <sstream>

HttpParser::HttpParser(const std::string &data) {
  std::istringstream stream{data};
  std::string line;
  while (std::getline(stream, line, '\n')) {
  }
}
