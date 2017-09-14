#include "socket.hpp"

extern "C" {
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
}
#include <cstring>
#include <fstream>
#include <memory>
#include <utility>
#include <vector>

std::string str_to_error(int error) {
  thread_local static char buf[1024];
#if (_POSIX_C_SOURCE >= 200112L) && !_GNU_SOURCE
  auto rc = ::strerror_r(error, buf, 1024);
  if (rc) throw std::runtime_error{"strerror_r failed to decode error"};
  return buf;
#else
  return ::strerror_r(error, buf, sizeof(buf));
#endif
}

Socket::Socket(Socket &&other) : d_socket{other.d_socket} { other.d_socket = 0; }

Socket &Socket::operator=(Socket &&other) {
  if (this != &other) {
    close();
    d_socket = other.d_socket;
    other.d_socket = -1;
  }
  return *this;
}

Socket::~Socket() { close(); }

void Socket::close() {
  if (d_socket > 0) {
    ::close(d_socket);
  }
}

Socket &Socket::bind(const sockaddr *addr_ptr, socklen_t len) {
  auto rc = ::bind(d_socket, addr_ptr, len);
  if (rc < 0) throw socket_exception{"Failed to bind"};
  return *this;
}

Socket &Socket::bind(const addrinfo &info) { return bind(info.ai_addr, info.ai_addrlen); }

ServerSocket &ServerSocket::listen(int backlog) {
  auto r = ::listen(d_socket, backlog);
  if (r) {
    throw socket_exception{str_to_error(errno)};
  }
  return *this;
}

std::unique_ptr<addrinfo, addrinfo_del> get_addr_info(const std::string &name,
                                                      const std::string &service,
                                                      const addrinfo &hints) {
  const char *name_cs = nullptr;
  if (!name.empty()) name_cs = name.c_str();
  const char *service_cs = nullptr;
  if (!service.empty()) service_cs = service.c_str();

  addrinfo *result_p;
  auto rcode = ::getaddrinfo(name_cs, service_cs, &hints, &result_p);
  if (rcode) throw socket_exception{::gai_strerror(rcode)};
  auto ptr = std::unique_ptr<addrinfo, addrinfo_del>(result_p);

  return ptr;
}

std::unique_ptr<addrinfo, addrinfo_del> get_addr_info(const std::string &name,
                                                      const std::string &service) {
  addrinfo hints = {};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  return get_addr_info(name, service, hints);
}

std::pair<std::vector<std::string>, std::vector<std::string>> addr_vec_from_addrinfo(
    const addrinfo &info) {
  std::vector<std::string> v4, v6;
  for (auto p = &info; p; p = p->ai_next) {
    void *addr;
    switch (p->ai_family) {
      case AF_INET: {  // IPv4
        char ipstr[INET_ADDRSTRLEN];
        auto *ipv4 = reinterpret_cast<sockaddr_in *>(p->ai_addr);
        addr = &(ipv4->sin_addr);
        ::inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
        v4.emplace_back(ipstr);
        break;
      }
      case AF_INET6: {  // IPv6
        char ipstr[INET6_ADDRSTRLEN];
        auto *ipv6 = reinterpret_cast<sockaddr_in6 *>(p->ai_addr);
        addr = &(ipv6->sin6_addr);
        ::inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
        v6.emplace_back(ipstr);
        break;
      }
      default:
        throw socket_exception{"Critical: unknown address family: " + std::to_string(p->ai_family)};
    }
  }
  return {std::move(v4), std::move(v6)};
}

hostent get_host_by_name(const std::string &name) {
  thread_local static std::vector<char> buffer{64};  // Thread-specific buffer
  hostent host = {};
  hostent *hp;
  int herr;
  for (;;) {
    int res = gethostbyname_r(name.c_str(), &host, buffer.data(), buffer.capacity(), &hp, &herr);
    if (res == ERANGE) {  // We're trusting the implementation not to return this too many times
      buffer.reserve(buffer.capacity() * 2);
    } else if (!res && hp) {
      return host;
    } else {
      switch (herr) {
        case HOST_NOT_FOUND:
          throw socket_exception{"Host not found"};
        case NO_DATA:
          throw socket_exception{"No data received"};
        case NO_RECOVERY:
          throw socket_exception{"A nonrecoverable name server error occurred"};
        case TRY_AGAIN:
          throw socket_exception{
              "A temporary error occurred on an authoritative name server.  Try again later."};
        default:
          throw socket_exception{"Unknown failure"};
      }
    }
  }
}

StreamSocket::StreamSocket(const std::string &address, in_port_t port) {
  addrinfo hints = {};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  auto info = get_addr_info(address, std::to_string(static_cast<int>(port)), hints);
  Socket::d_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
  connect(*info);
}

StreamSocket &StreamSocket::connect(const sockaddr *addr_ptr, socklen_t len) {
  auto rc = ::connect(Socket::d_socket, addr_ptr, len);
  if (rc < 0) {
    throw socket_exception{str_to_error(errno)};
  }
  return *this;
}

StreamSocket &StreamSocket::connect(const addrinfo &info) {
  return connect(info.ai_addr, info.ai_addrlen);
}

StreamSocket &StreamSocket::send(const char *buffer_p, size_t sz, int flags) {
  auto to_send = sz;
  while (to_send) {
    auto sent = ::send(Socket::d_socket, buffer_p, to_send, flags);
    if (sent < 0) {
      throw socket_exception{str_to_error(errno)};
    }
    if (static_cast<size_t>(sent) > to_send) {
      throw socket_exception{"Sent more than requested!"};
    }
    to_send -= static_cast<size_t>(sent);
    buffer_p += sent;
  }
  return *this;
}

StreamSocket &StreamSocket::send(const std::string &data, int flags) {
  return send(data.data(), data.size(), flags);
}

StreamSocket &StreamSocket::send(std::ifstream &input, int flags) {
  static thread_local char buf[8 * 1024];
  while (input) {
    input.read(buf, sizeof(buf));
    if (input.gcount() > 0 && input.good() && static_cast<size_t>(input.gcount()) <= sizeof(buf)) {
      send(buf, static_cast<size_t>(input.gcount()), flags);
    } else if (input.eof()) {
      send(buf, static_cast<size_t>(input.gcount()), flags);
      return *this;
    }
  }
  if (input.fail()) throw socket_exception{"Failure reading from ifstream"};
  return *this;
}

std::string StreamSocket::recv(int flags) {
  thread_local static char raw_buf[64 * 1024];  // 64 KiB
  constexpr auto raw_buf_sz = sizeof(raw_buf) / sizeof(raw_buf[0]);

  auto received = ::recv(Socket::d_socket, raw_buf, raw_buf_sz, flags);
  if (received < 0) {
    throw socket_exception{str_to_error(errno)};
  }

  return {raw_buf, static_cast<size_t>(received)};
}

sockaddr_storage StreamSocket::get_peer_name() {
  struct sockaddr_storage addr = {};
  auto len = static_cast<socklen_t>(sizeof(addr));

  auto rc = ::getpeername(Socket::d_socket, reinterpret_cast<sockaddr *>(&addr), &len);

  if (rc) {
    throw socket_exception{str_to_error(errno)};
  }

  return addr;
}

PeerInfo StreamSocket::get_peer_info() {
  auto addr = get_peer_name();
  in_port_t port;
  char ipstr[INET6_ADDRSTRLEN] = {0};
  if (addr.ss_family == AF_INET) {
    auto s = reinterpret_cast<sockaddr_in *>(&addr);
    port = ntohs(s->sin_port);
    inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr));
  } else {  // AF_INET6
    auto s = reinterpret_cast<sockaddr_in6 *>(&addr);
    port = ntohs(s->sin6_port);
    inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof(ipstr));
  }
  return {port, std::string{ipstr}};
}

StreamServerSocket::StreamServerSocket(in_port_t port) {
  addrinfo hints = {};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  auto info = get_addr_info("", std::to_string(static_cast<int>(port)), hints);
  Socket::d_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
  Socket::bind(info->ai_addr, info->ai_addrlen);
}

StreamServerSocket::StreamServerSocket(const std::string &address, in_port_t port) {
  addrinfo hints = {};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  auto info = get_addr_info(address, std::to_string(static_cast<int>(port)), hints);
  Socket::d_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
  Socket::bind(*info);
}

StreamServerSocket::StreamServerSocket(ServerSocket &&sock) : ServerSocket{std::move(sock)} {}

StreamSocket StreamServerSocket::accept() {
  sockaddr_storage their_addr = {};
  socklen_t addr_size = sizeof(their_addr);
  auto socket_descriptor =
      ::accept(Socket::d_socket, reinterpret_cast<sockaddr *>(&their_addr), &addr_size);
  if (socket_descriptor < 0) {
    throw socket_exception{str_to_error(errno)};
  }

  return StreamSocket{socket_descriptor};
}
