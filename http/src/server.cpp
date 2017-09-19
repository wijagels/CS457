#include "server.hpp"
#include "http.hpp"
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <future>
#include <ios>
#include <regex>
#include <system_error>
#include <unordered_map>

#if __has_include(<filesystem>)
#include <filesystem>
namespace {
namespace stdfs = stdfs;
}
#else
#include <experimental/filesystem>
namespace {
namespace stdfs = std::experimental::filesystem;
}
#endif

static const auto g_regex_flags = std::regex_constants::ECMAScript | std::regex_constants::optimize;
static const std::regex g_http_msg_end{"(?:\r\n\r\n)|(?:\r\r)|(?:\n\n)", g_regex_flags};

HttpServer::HttpServer(uint16_t port, stdfs::path base_path)
    : d_socket{port}, d_base_path{std::move(base_path)} {
  if (!stdfs::is_directory(base_path)) {
    std::cout << "Trashttpd started on the any address, port " << port << '\n';
  } else {
    std::cerr << "Nonexistent base path, exiting.\n";
    exit(1);
  }
}

HttpServer::HttpServer(const std::string &address, uint16_t port, stdfs::path base_path)
    : d_socket{address, port}, d_base_path{std::move(base_path)} {
  if (!stdfs::is_directory(base_path)) {
    std::cout << "Trashttpd started on " << address << ":" << port << '\n';
  } else {
    std::cerr << "Nonexistent base path, exiting.\n";
    exit(1);
  }
}

void HttpServer::listen(int backlog) { d_socket.listen(backlog); }

void HttpServer::accept_connections() {
  auto acceptor = [this](StreamSocket &&conn) { conn_acceptor(std::move(conn)); };
  for (;;) {
    std::thread(acceptor, d_socket.accept()).detach();
  }
}

void HttpServer::send_response(HttpResponse &response, StreamSocket &conn) {
  response.d_response_headers.emplace_back("Date", http_time());
  response.d_response_headers.emplace_back("Connection", "keep-alive");
  response.d_response_headers.emplace_back("Server", "trashttpd/1.0.0 (GNU+Linux)");

  conn.cork();  // Performance tweak, ensure the headers don't get sent if they don't fill a frame.
  conn.send(response.make_header());
  if (response.d_status_code < 400) {
    conn.send_file(response.d_file.fd(), response.d_length);
  } else {
    conn.send(response.d_error_str);
  }
  conn.uncork();
}

void HttpServer::conn_acceptor(StreamSocket &&conn) {
  try {
    for (;;) {
      try {
        auto s = conn.recv();
        HttpParser parser{s};
        auto resp = serve_get(parser, conn.get_peer_info());
        send_response(resp, conn);
      } catch (const http_exception &e) {
        std::cerr << e.what() << std::endl;
        HttpResponse res{400, {}, {}};
        res.d_error_str = e.what();
        conn.send(res.make_header());
      } catch (const socket_exception &e) {
        std::cerr << "Socket error " << e.what() << std::endl;
        break;
      } catch (const socket_closed &) {
        // Normal behavior, connection was closed.
        break;
      } catch (const std::runtime_error &e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        HttpResponse res{500, {{"Content-Length", "0"}}, {}};
        conn.send(res.make_header());
        break;
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "**Critical** Double exception: " << e.what();
  }
}

HttpResponse HttpServer::serve_get(const HttpParser &parser, const PeerInfo &peer) {
  auto path = parser.path();
  auto p = stdfs::path{path};
  if (!p.is_absolute()) {
    return {400, {{"Content-Length", "0"}}, {}};
  }
  auto full_path = d_base_path / p;
  if (!stdfs::exists(full_path) || stdfs::is_directory(full_path)) {
    return {404, {{"Content-Length", "0"}}, {}};
  }

  auto ext = full_path.extension().native();
  if (ext.size()) {
    // Trim leading dot
    ext = ext.substr(1);
  }
  d_logger.log_get(path, peer.address, peer.port);

  auto file_size = stdfs::file_size(full_path);
  auto last_modified = time_to_http(stdfs::last_write_time(full_path));

  std::vector<std::pair<std::string, std::string>> headers{
      {"Content-Type", d_mimedb.mime_of_ext(ext)},
      {"Content-Length", std::to_string(file_size)},
      {"Last-Modified", last_modified}};

  File file{full_path.native(), O_RDONLY};
  if (file.fd() < 0) {
    const std::string error_str = "Server overloaded";
    auto ret = HttpResponse{503, {{"Content-Length", std::to_string(error_str.size())}}, {}};
    ret.d_error_str = error_str;
    return ret;
  }

  return {200, headers, std::move(file), file_size};
}
