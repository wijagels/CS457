#ifndef HTTP_HPP_
#define HTTP_HPP_
#include <string>
#include <unordered_map>

class HttpParser {
 public:
  HttpParser(const std::string &data);

  void parse_headers(const std::string &data);

  inline const auto &headers() const { return d_headers; }

 private:
  std::unordered_map<std::string, std::string> d_headers;
  std::string d_headline;
};

/*
 * Returns the current time in RFC 2616 section 3.3.1 format
 */
std::string http_time();

#endif
