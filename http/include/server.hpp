#ifndef SERVER_HPP_
#define SERVER_HPP_
#include "logger.hpp"
#include "socket.hpp"
#include "http.hpp"
#include "mimedb.hpp"
#include <boost/filesystem/path.hpp>
#include <cstdint>
#include <string>
#include <vector>
#include <fstream>

class HttpServer {
 public:
  HttpServer(uint16_t port);
  HttpServer(const std::string &address, uint16_t port);

  void listen(int backlog = 10);

  void accept_connections();

 private:
  void conn_acceptor(StreamSocket &&conn);
  HttpResponse serve_get(const HttpParser &parser, const PeerInfo &peer);

  StreamServerSocket d_socket;
  boost::filesystem::path d_base_path;
  Logger d_logger;
  MimeDb d_mimedb;
};

#endif
