#include "http.hpp"
#include <chrono>
#include <ctime>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

static constexpr auto g_http_ver = "HTTP/1.1";
static constexpr auto g_CRLF = "\r\n";

/*
 * Reason phrases from rfc2616
 */
static const std::unordered_map<uint_fast16_t, const char *> g_status_reason_map{
    {100, "Continue"},
    {101, "Switching Protocols"},
    {200, "OK"},
    {201, "Created"},
    {202, "Accepted"},
    {203, "Non-Authoritative Information"},
    {204, "No Content"},
    {205, "Reset Content"},
    {206, "Partial Content"},
    {300, "Multiple Choices"},
    {301, "Moved Permanently"},
    {302, "Found"},
    {303, "See Other"},
    {304, "Not Modified"},
    {305, "Use Proxy"},
    {307, "Temporary Redirect"},
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {402, "Payment Required"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {406, "Not Acceptable"},
    {407, "Proxy Authentication Required"},
    {408, "Request Time-out"},
    {409, "Conflict"},
    {410, "Gone"},
    {411, "Length Required"},
    {412, "Precondition Failed"},
    {413, "Request Entity Too Large"},
    {414, "Request-URI Too Large"},
    {415, "Unsupported Media Type"},
    {416, "Requested range not satisfiable"},
    {417, "Expectation Failed"},
    {500, "Internal Server Error"},
    {501, "Not Implemented"},
    {502, "Bad Gateway"},
    {503, "Service Unavailable"},
    {504, "Gateway Time-out"},
    {505, "HTTP Version not supported"}};

static const std::unordered_map<std::string, HttpMethod> g_http_method_map{
    {"GET", HttpMethod::GET},         {"HEAD", HttpMethod::HEAD},
    {"POST", HttpMethod::POST},       {"PUT", HttpMethod::PUT},
    {"DELETE", HttpMethod::DELETE},   {"CONNECT", HttpMethod::CONNECT},
    {"OPTIONS", HttpMethod::OPTIONS}, {"TRACE", HttpMethod::TRACE},
    {"PATCH", HttpMethod::PATCH}};

HttpParser::HttpParser(const std::string &data) { parse_headers(data); }

void HttpParser::parse_headers(const std::string &data) {
  std::istringstream stream{data};
  std::string line;
  std::getline(stream, line);
  d_request_line = line;
  while (std::getline(stream, line) && line != "\r") {
    auto pos = line.find(':');
    std::string title{line, 0, pos};
    std::string value{line, pos + 1};
    d_headers.emplace(std::move(title), std::move(value));
  }
}

std::string HttpParser::path() const {
  auto start_pos = d_request_line.find(" ");
  if (start_pos == std::string::npos) throw http_exception{"Invalid headline encountered"};
  auto end_pos = d_request_line.find(" ", start_pos + 1);
  if (end_pos == std::string::npos) throw http_exception{"Invalid headline encountered"};
  return d_request_line.substr(start_pos + 1, (end_pos - start_pos - 1));
}

HttpResponse::HttpResponse(uint_fast16_t status_code, std::string reason_phrase,
                           std::vector<std::pair<std::string, std::string>> response_headers,
                           File file, size_t length)
    : d_status_code{status_code},
      d_reason_phrase{std::move(reason_phrase)},
      d_response_headers{std::move(response_headers)},
      d_file{std::move(file)},
      d_length{length} {}

HttpResponse::HttpResponse(uint_fast16_t status_code,
                           std::vector<std::pair<std::string, std::string>> response_headers,
                           File file, size_t length)
    : HttpResponse{status_code, g_status_reason_map.at(status_code), std::move(response_headers),
                   std::move(file), length} {}

std::string HttpResponse::make_header() {
  std::ostringstream os;
  os << g_http_ver << " " << d_status_code << " " << d_reason_phrase << g_CRLF;
  for (const auto &header : d_response_headers) {
    os << header.first << ": " << header.second << g_CRLF;
  }
  os << g_CRLF;
  return os.str();
}

std::string http_time() { return time_to_http(std::chrono::system_clock::now()); }
