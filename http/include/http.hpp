#ifndef HTTP_HPP_
#define HTTP_HPP_
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

struct http_exception : std::runtime_error {
  explicit http_exception(const char *what_arg) : std::runtime_error{what_arg} {}
  explicit http_exception(const std::string &what_arg) : std::runtime_error{what_arg} {}
};

enum class HttpMethod { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, PATCH };

class HttpParser {
 public:
  HttpParser(const std::string &data);

  void parse_headers(const std::string &data);

  inline const auto &headers() const { return d_headers; }

  /*
   * Extract just the path from the request-line
   */
  std::string path() const;

 private:
  std::unordered_map<std::string, std::string> d_headers;
  std::string d_request_line;
};

/*
 * Encapsulates HTTP response headers.
 * Performs translation of status codes to reason phrases if no reason_phrase is supplied.
 * Useful as a return object.
 * Move-only type due to ifstream ownership.
 */
struct HttpResponse {
  HttpResponse(uint_fast16_t response_code, std::string reason_phrase,
               std::vector<std::pair<std::string, std::string>> response_headers,
               std::ifstream &&body_stream);

  HttpResponse(uint_fast16_t response_code,
               std::vector<std::pair<std::string, std::string>> response_headers,
               std::ifstream &&body_stream);

  std::string make_header();

  uint_fast16_t d_status_code;
  std::string d_reason_phrase;
  std::vector<std::pair<std::string, std::string>> d_response_headers;
  std::ifstream d_body_stream;
  // Error string for status codes >= 400
  std::string d_error_str;
};

/*
 * Returns the current time in RFC 2616 section 3.3.1 format
 */
std::string http_time();

#endif
