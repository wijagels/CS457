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
}
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

struct socket_exception : std::runtime_error {
  explicit socket_exception(const char *what_arg) : std::runtime_error{what_arg} {}
  explicit socket_exception(const std::string &what_arg) : std::runtime_error{what_arg} {}
};

struct addrinfo_del {
  auto operator()(addrinfo *ptr) { freeaddrinfo(ptr); }
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

class Socket {
 public:
  Socket(int type = SOCK_STREAM, int protocol = 0);
  ~Socket();
  Socket(const Socket &) = delete;
  Socket(Socket &&) = default;
  Socket &operator=(const Socket &) = delete;
  Socket &operator=(Socket &&) = default;

  /*
   * Fetches address of hostname and sets the socket to listen on that address on the supplied port
   */
  void bind_address(const std::string &hostname, uint16_t port);

  /*
   * Binds socket to listen on any interface at the specified port
   */
  void bind_all(uint16_t port);

  /*
   * Wrapper around bind(2)
   */
  void bind(sockaddr *addr_ptr, socklen_t len);

  /*
   * Wraps listen(2).
   */
  void listen(int backlog);

 protected:
  int d_descriptor4;
  int d_descriptor6;
};

#endif
