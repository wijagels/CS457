#include "server.hpp"
#include "http.hpp"
#include <algorithm>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <cstdint>
#include <fstream>
#include <future>
#include <ios>
#include <system_error>
#include <unordered_map>

HttpServer::HttpServer(uint16_t port) : d_socket{port} {}

HttpServer::HttpServer(const std::string &address, uint16_t port) : d_socket{address, port} {}

void HttpServer::listen(int backlog) { d_socket.listen(backlog); }

void HttpServer::accept_connections() {
  auto acceptor = [this](StreamSocket &&conn) { return conn_acceptor(std::move(conn)); };
  for (;;) {
    try {
      std::async(std::launch::async, acceptor, d_socket.accept());
    } catch (socket_exception &e) {
      std::cerr << e.what() << std::endl;
    }
  }
}

void HttpServer::conn_acceptor(StreamSocket &&conn) {
  try {
    auto s = conn.recv();
    HttpParser parser{s};
    auto res = serve_get(parser, conn.get_peer_info());
    conn.send(res.make_header());
    if (res.d_status_code < 400) {
      conn.send(res.d_body_stream);
    } else {
      conn.send(res.d_error_str);
    }
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
    HttpResponse res{500, {}, {}};
    conn.send(res.make_header());
  }
}

HttpResponse HttpServer::serve_get(const HttpParser &parser, const PeerInfo &peer) {
  try {
    auto path = parser.path();
    auto p = boost::filesystem::path{path};
    if (!p.is_absolute()) {
      return {400, {}, {}};
    }
    auto full_path = boost::filesystem::current_path();
    full_path /= p;
    if (!boost::filesystem::exists(full_path) || boost::filesystem::is_directory(full_path)) {
      return {404, {}, {}};
    }

    auto ext = full_path.extension().native();
    std::vector<std::pair<std::string, std::string>> headers;
    headers.push_back({"Content-Type", d_mimedb.mime_of_ext(ext)});
    d_logger.log_get(path, peer.address, peer.port);

    std::ifstream file{full_path.native(), std::ios::ate};
    // Determine size
    auto length = file.tellg();
    file.seekg(0);
    headers.push_back({"Content-Length", std::to_string(length)});
    std::cout << headers.back().second << "\n";

    return {200, headers, std::move(file)};

  } catch (const http_exception &e) {
    // Assume in all cases that it's the user's fault
    HttpResponse resp{400, {}, {}};
    resp.d_error_str = e.what();
    return resp;
  }
}
