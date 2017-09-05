#ifndef HTTP_HPP_
#define HTTP_HPP_
#include <string>
#include <unordered_map>

class HttpParser {
 public:
  HttpParser(const std::string &data);

  inline std::unordered_map<std::string, std::string> &headers() { return d_headers; }

 private:
  std::unordered_map<std::string, std::string> d_headers;
};

#endif
