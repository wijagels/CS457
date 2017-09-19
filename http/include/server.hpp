#ifndef SERVER_HPP_
#define SERVER_HPP_
#include "file.hpp"
#include "http.hpp"
#include "logger.hpp"
#include "mimedb.hpp"
#include "socket.hpp"
#include <cstdint>
#include <fstream>
#include <string>

/*
 * Come on libstdc++ devs, hurry up and finish this.
 */
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

class HttpServer {
 public:
  HttpServer(uint16_t port, stdfs::path base_path = stdfs::current_path() / "www");
  HttpServer(const std::string &address, uint16_t port,
             stdfs::path base_path = stdfs::current_path() / "www");

  /**
   * Begin listening on the socket.
   * Does not accept connections, they simply will queue up until accept_connections() accepts them.
   * MT-Safety: unsafe, do not call concurrently on the same object
   */
  void listen(int backlog = 1000);

  /**
   * Accepts connections on the owned socket.
   * When a new connection is accepted, this spawns a new thread and calls conn_acceptor on the new
   * connection.
   * MT-Safety: safe to call from multiple threads, even on the same object.
   */
  void accept_connections();

 private:
  void send_response(HttpResponse &response, StreamSocket &conn);

  /**
   * Accepts a StreamSocket connection that has been created from StreamServerSocket::accept().
   * MT-Safety: safe since StreamSockets are move only, so no other thread may use the socket.
   */
  void conn_acceptor(StreamSocket &&conn);

  /**
   * Constructs a HttpResponse from a request.
   * MT-Safety: Safe; Data race: stderr
   */
  HttpResponse serve_get(const HttpParser &parser, const PeerInfo &peer);

  StreamServerSocket d_socket;
  stdfs::path d_base_path;
  Logger d_logger;
  MimeDb d_mimedb;
};

#endif
