#include "server.hpp"
#include "http.hpp"
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <cstdint>
#include <fstream>
#include <future>
#include <ios>
#include <regex>
#include <system_error>
#include <unordered_map>

static const std::regex g_http_msg_end{"(?:\r\n\r\n)|(?:\r\r)|(?:\n\n)"};

HttpServer::HttpServer(uint16_t port, boost::filesystem::path base_path)
    : d_socket{port}, d_base_path{std::move(base_path)} {}

HttpServer::HttpServer(const std::string &address, uint16_t port, boost::filesystem::path base_path)
    : d_socket{address, port}, d_base_path{std::move(base_path)} {}

void HttpServer::listen(int backlog) { d_socket.listen(backlog); }

void HttpServer::accept_connections() {
  auto acceptor = [this](StreamSocket &&conn) { conn_acceptor(std::move(conn)); };
  for (;;) {
      std::thread(acceptor, d_socket.accept()).detach();
  }
}

void HttpServer::send_response(HttpResponse &response, StreamSocket &conn) {
  conn.send(response.make_header());
  if (response.d_status_code < 400) {
    conn.send(response.d_body_stream);
  } else {
    conn.send(response.d_error_str);
  }
}

void HttpServer::conn_acceptor(StreamSocket &&conn) {
  try {
    for (;;) {
      try {
        auto s = conn.recv();
        HttpParser parser{s};
        auto resp = serve_get(parser, conn.get_peer_info());
        send_response(resp, conn);
        // auto resp = HttpResponse{200, {{"Content-length", "102"}},
        // std::ifstream{"www/test.html"}};
        // send_response(resp, conn);
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
  auto p = boost::filesystem::path{path};
  if (!p.is_absolute()) {
    return {400, {{"Connection", "keep-alive"}, {"Content-Length", "0"}}, {}};
  }
  auto full_path = d_base_path / p;
  if (!boost::filesystem::exists(full_path) || boost::filesystem::is_directory(full_path)) {
    return {404, {{"Connection", "keep-alive"}, {"Content-Length", "0"}}, {}};
  }

  auto ext = full_path.extension().native();
  if (ext.size()) {
    // Trim leading dot
    ext = ext.substr(1);
  }
  d_logger.log_get(path, peer.address, peer.port);

  std::ifstream file{full_path.native(), std::ios::ate};
  // Determine size
  auto length = file.tellg();
  file.seekg(0);

  std::vector<std::pair<std::string, std::string>> headers{
      {"Connection", "keep-alive"},
      {"Content-Type", d_mimedb.mime_of_ext(ext)},
      {"Content-Length", std::to_string(length)}};

  return {200, headers, std::move(file)};
}
