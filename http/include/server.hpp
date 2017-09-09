#ifndef SERVER_HPP_
#define SERVER_HPP_
#include "socket.hpp"
#include <atomic>
#include <boost/filesystem/path.hpp>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

class HttpServer {
 public:
  HttpServer(uint16_t port);
  HttpServer(const std::string &address, uint16_t port);

  void listen(int backlog = 10);

  void accept_connections();

  void serve_get(const std::string &path);
 private:
  void conn_acceptor(StreamSocket &&conn);


  StreamServerSocket d_socket;
  boost::filesystem::path d_base_path;
  std::unordered_map<std::string, std::atomic_int_fast32_t> d_counts;
  std::mutex d_write_lock;
};

#endif
