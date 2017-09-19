#include "socket.hpp"

extern "C" {
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <unistd.h>
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
    ::shutdown(d_socket, 2);
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
  hints.ai_family = AF_UNSPEC;
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
  // Don't send a signal in the event of hangup, it's already handled
  flags |= MSG_NOSIGNAL;

  auto to_send = sz;
  while (to_send) {
    auto sent = ::send(Socket::d_socket, buffer_p, to_send, flags);
    if (sent < 0) {
      throw socket_exception{str_to_error(errno)};
    } else if (static_cast<size_t>(sent) > to_send) {
      throw socket_exception{"Sent more than requested!"};
    } else if (!sent) {
      throw socket_exception{"Nothing was sent!"};
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
  char buf[4 * 1024];
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

StreamSocket &StreamSocket::send_file(int fd, size_t count) {
  off_t offset = 0;
  while (count) {
    auto sent = ::sendfile(Socket::d_socket, fd, &offset, count);
    if (sent < 0) {
      throw socket_exception{"Sendfile failed: " + str_to_error(errno)};
    }
    if (static_cast<size_t>(sent) > count) {
      throw socket_exception{"Sendfile sent more than requested"};
    }
    count -= static_cast<size_t>(sent);
  }
  return *this;
}

std::string StreamSocket::recv(int flags) {
  char raw_buf[4 * 1024];  // 4 KiB

  auto received = ::recv(Socket::d_socket, raw_buf, sizeof(raw_buf), flags);
  if (received == 0) {
    throw socket_closed{"Socket is closed"};
  } else if (received < 0) {
    throw socket_exception{str_to_error(errno)};
  }

  return {raw_buf, static_cast<size_t>(received)};
}

std::string StreamSocket::recv_msg(const std::regex &delimiter, int flags) {
  // if (std::regex_search(d_unsent_buf, results, delimiter)) {
  //   auto end_idx = results.position() + results.length();
  //   auto ret = d_unsent_buf.substr(0, end_idx);
  //   d_unsent_buf = d_unsent_buf.substr(end_idx + 1, d_unsent_buf.size() - end_idx);
  //   return ret;
  // }

  std::match_results<std::string::const_iterator> results;
  std::string ret = d_unsent_buf;
  std::string buf = d_unsent_buf;

  while (!std::regex_search(buf, results, delimiter)) {
    ret.append(buf);
    buf = recv(flags);
  }

  auto end_idx_l = results.position() + results.length();
  if (end_idx_l < 0) {
    throw socket_exception{"Ending index of regex search is negative"};
  }
  auto end_idx = static_cast<size_t>(end_idx_l);
  ret.append(d_unsent_buf.substr(0, end_idx));
  d_unsent_buf = d_unsent_buf.substr(end_idx + 1, d_unsent_buf.size() - end_idx);

  return ret;
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

void StreamSocket::cork() { Socket::set_option(IPPROTO_TCP, TCP_CORK, 1); }
void StreamSocket::uncork() { Socket::set_option(IPPROTO_TCP, TCP_CORK, 0); }

StreamServerSocket::StreamServerSocket(in_port_t port) {
  addrinfo hints = {};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  auto info = get_addr_info("", std::to_string(static_cast<int>(port)), hints);
  Socket::d_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
  Socket::set_option(SOL_SOCKET, SO_REUSEADDR, 1);
  Socket::set_option(IPPROTO_TCP, TCP_NODELAY, 1);
  Socket::bind(*info);
}

StreamServerSocket::StreamServerSocket(const std::string &address, in_port_t port) {
  addrinfo hints = {};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  auto info = get_addr_info(address, std::to_string(static_cast<int>(port)), hints);
  Socket::d_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
  Socket::set_option(SOL_SOCKET, SO_REUSEADDR, 1);
  Socket::set_option(IPPROTO_TCP, TCP_NODELAY, 1);
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
