#ifndef SERVER_HPP_
#define SERVER_HPP_
#include "http.hpp"
#include "logger.hpp"
#include "mimedb.hpp"
#include "socket.hpp"
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

class HttpServer {
 public:
  HttpServer(uint16_t port,
             boost::filesystem::path base_path = boost::filesystem::current_path() /
                                                 boost::filesystem::path{"www"});
  HttpServer(const std::string &address, uint16_t port,
             boost::filesystem::path base_path = boost::filesystem::current_path() /
                                                 boost::filesystem::path{"www"});

  void listen(int backlog = 1000);

  void accept_connections();

 private:
  void send_response(HttpResponse &response, StreamSocket &conn);
  void conn_acceptor(StreamSocket &&conn);
  HttpResponse serve_get(const HttpParser &parser, const PeerInfo &peer);

  StreamServerSocket d_socket;
  boost::filesystem::path d_base_path;
  Logger d_logger;
  MimeDb d_mimedb;
};

#endif
