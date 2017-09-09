#include "http.hpp"
#include <ctime>          
#include <boost/algorithm/string/trim.hpp> 
#include <chrono>                          
#include <ctime>                           
#include <iomanip>                         
#include <iostream>                        
#include <string>                          
#include <unordered_map>                   
#include <utility>                         

HttpParser::HttpParser(const std::string &data) { parse_headers(data); }

void HttpParser::parse_headers(const std::string &data) {
  std::istringstream stream{data};
  std::string line;
  std::getline(stream, line);
  d_headline = line;
  while (std::getline(stream, line) && line != "\r") {
    auto pos = line.find(':');
    std::string title{line, 0, pos};
    std::string value{line, pos + 1};
    boost::trim(title);
    boost::trim(value);
    d_headers.emplace(std::move(title), std::move(value));
  }
  for (const auto &x : d_headers) {
    std::cout << x.first << ", " << x.second << std::endl;
  }
}

std::string http_time() {
  auto now = std::chrono::system_clock::now();
  auto now_c = std::chrono::system_clock::to_time_t(now);
  std::tm out;
  gmtime_r(&now_c, &out);
  std::ostringstream os;
  os << std::put_time(&out, "%a, %d %b %Y %H:%M:%S %Z");
  return os.str();
}
