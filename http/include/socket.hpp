#ifndef SOCKET_HPP_
#define SOCKET_HPP_
/*
 * This takes inspiration from Practical Sockets by Jeff Donahoo.
 * http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/practical/
 * However, this library ignores the windows platform and leverages more modern C++ concepts.
 * Additionally, this library will not hand out raw owning pointers because that would be
 * irresponsible for a C++14 library.
 * I give Jeff a pass because he's targeting VC++ which is often not even C++11 compliant.
 * There's also an attempt at supporting IPv6 because why not.
 */

extern "C" {
#include <netdb.h>
#include <netinet/in.h>
#include <stddef.h>
#include <sys/socket.h>
}

#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <regex>

struct socket_exception : std::runtime_error {
  explicit socket_exception(const char *what_arg) : std::runtime_error{what_arg} {}
  explicit socket_exception(const std::string &what_arg) : std::runtime_error{what_arg} {}
};

struct socket_closed : std::runtime_error {
  explicit socket_closed(const char *what_arg) : std::runtime_error{what_arg} {}
  explicit socket_closed(const std::string &what_arg) : std::runtime_error{what_arg} {}
};

struct addrinfo_del {
  auto operator()(addrinfo *ptr) {
    if (ptr) {
      freeaddrinfo(ptr);
    }
  }
};

struct PeerInfo {
  in_port_t port;
  std::string address;
};

/*
 * MT-Safe wrapper for gethostbyname_r
 * You probably want to use get_addr_info instead.
 */
hostent get_host_by_name(const std::string &);

/*
 * Calls getaddrinfo with the provided parameters and returns a smart pointer to the addrinfo
 * struct.
 * The caller should not call freeaddrinfo().
 * See std::unique_ptr for lifetime information.
 * If name or service are empty, they will be replaced with a nullptr when calling glibc.
 */
std::unique_ptr<addrinfo, addrinfo_del> get_addr_info(const std::string &name,
                                                      const std::string &service,
                                                      const addrinfo &hints);

std::unique_ptr<addrinfo, addrinfo_del> get_addr_info(const std::string &name,
                                                      const std::string &service);

/*
 * Iterate over a addrinfo structure, and decode all addresses.
 * Returned as a pair of vectors, where the first vector is ipv4 addresses and the second is ipv6
 * addresses.
 * http://beej.us/guide/bgnet/output/html/multipage/syscalls.html#getaddrinfo
 */
std::pair<std::vector<std::string>, std::vector<std::string>> addr_vec_from_addrinfo(
    const addrinfo &info);

std::string str_to_error(int error);

class Socket {
 public:
  Socket() = default;
  ~Socket();
  constexpr explicit Socket(int sock) noexcept : d_socket{sock} {}
  Socket(const Socket &) = delete;
  Socket(Socket &&);
  Socket &operator=(const Socket &) = delete;
  Socket &operator=(Socket &&);

  void close();

  /*
   * Wrapper around bind(2)
   */
  Socket &bind(const sockaddr *addr_ptr, socklen_t len);

  /*
   * Convenience function equivalent to bind(info.ai_addr, info.ai_addrlen)
   */
  Socket &bind(const addrinfo &info);

 protected:
  int d_socket = -1;
};

struct StreamSocket : Socket {
  /*
   * Take ownership over a socket file descriptor
   */
  constexpr explicit StreamSocket(int sock) noexcept : Socket{sock} {}

  /*
   * Initialize socket, connect to the given address and port
   */
  StreamSocket(const std::string &address, in_port_t port);

  /*
   * Wrapper around connect()
   * http://beej.us/guide/bgnet/output/html/multipage/syscalls.html#connect
   */
  StreamSocket &connect(const sockaddr *addr_ptr, socklen_t len);

  /*
   * Convenience function equivalent to connect(info.ai_addr, info.ai_addrlen)
   */
  StreamSocket &connect(const addrinfo &info);

  /*
   * Send buffer of size sz over socket.
   */
  StreamSocket &send(const char *buffer_p, size_t sz, int flags = 0);

  /*
   * Send string over socket.
   * Requires connection to be first established.
   */
  StreamSocket &send(const std::string &data, int flags = 0);

  /*
   * Send data from an input stream until exhausted
   */
  StreamSocket &send(std::ifstream &input, int flags = 0);

  /*
   * Fetches a string from the socket.
   * Uses a thread-local buffer to avoid exploding the stack or making needless heap allocations
   */
  std::string recv(int flags = 0);

  std::string recv_msg(const std::regex &delimiter, int flags = 0);

  /*
   * Wrapper around getpeername()
   * http://beej.us/guide/bgnet/output/html/multipage/getpeernameman.html
   */
  sockaddr_storage get_peer_name();

  /*
   * Higher level wrapper around getpeername, decodes the port and IP into strings.
   */
  PeerInfo get_peer_info();
};

struct DatagramSocket : Socket {
  DatagramSocket &send_to(const std::string &data, const sockaddr &dest, socklen_t dest_len,
                          int flags = 0);
  std::string recv_from(const std::string &data, const sockaddr &dest, socklen_t dest_len,
                        int flags = 0);
};

struct ServerSocket : Socket {
  /*
   * Wraps listen(2).
   */
  ServerSocket &listen(int backlog);
};

struct StreamServerSocket : ServerSocket {
  /*
   * Listen on any interface
   */
  StreamServerSocket(in_port_t port);

  /*
   * Listen on a specific address
   */
  StreamServerSocket(const std::string &address, in_port_t port);

  /*
   * Promote a server socket to a stream server socket.
   */
  StreamServerSocket(ServerSocket &&sock);

  /*
   * Accept a connection and return a StreamSocket
   * http://beej.us/guide/bgnet/output/html/multipage/syscalls.html#accept
   */
  StreamSocket accept();
};

#endif
