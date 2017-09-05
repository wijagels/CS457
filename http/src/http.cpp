#include "http.hpp"
#include <sstream>
#include <string>
#include <unordered_map>

HttpParser::HttpParser(const std::string &data) {
  std::istringstream stream{data};
  std::string line;
  while (std::getline(stream, line, '\n')) {
  }
}
