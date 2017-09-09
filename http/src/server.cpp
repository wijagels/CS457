#include "server.hpp"
#include "http.hpp"
#include <algorithm>
#include <cstdint>
#include <future>
#include <iostream>
#include <stdexcept>
#include <system_error>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

HttpServer::HttpServer(uint16_t port) : d_socket{port} {}

HttpServer::HttpServer(const std::string &address, uint16_t port) : d_socket{address, port} {}

void HttpServer::listen(int backlog) { d_socket.listen(backlog); }

void HttpServer::accept_connections() {
  auto acceptor = [this](StreamSocket &&conn) { return conn_acceptor(std::move(conn)); };
  for (;;) {
    try {
      std::async(std::launch::async, acceptor, std::move(d_socket.accept()));
    } catch (socket_exception &e) {
      std::cerr << e.what() << std::endl;
    }
  }
}

void HttpServer::conn_acceptor(StreamSocket &&conn) {
  try {
    auto s = conn.recv();
    HttpParser{s};
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
}

void HttpServer::serve_get(const std::string &path) {
  auto p = boost::filesystem::path{path};
  p = p.lexically_proximate(boost::filesystem::current_path());
  std::cout << p << std::endl;
}
