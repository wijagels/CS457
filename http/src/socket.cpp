#include "socket.hpp"

extern "C" {
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
}
#include <cstdio>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>

Socket::Socket(int type, int protocol) {
  d_descriptor4 = socket(AF_INET, type, protocol);
  d_descriptor6 = socket(AF_INET6, type, protocol);
}

Socket::~Socket() {
  close(d_descriptor4);
  close(d_descriptor6);
}

void Socket::bind_address(const std::string &hostname, uint16_t port) {
  auto info = get_addr_info(hostname, std::to_string(static_cast<int>(port)));
  bind(info->ai_addr, info->ai_addrlen);
}

void Socket::bind_all(uint16_t port) {
  addrinfo hints = {};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  auto info = get_addr_info("", std::to_string(static_cast<int>(port)), hints);
  bind(info->ai_addr, info->ai_addrlen);
}

void Socket::bind(sockaddr *addr_ptr, socklen_t len) {
  auto rc = ::bind(d_descriptor4, addr_ptr, len);
  if (rc < 0) throw socket_exception{"Failed to bind"};
}

void Socket::listen(int backlog) {
  auto r = ::listen(d_descriptor4, backlog);
  if (r) {
    auto err = errno;  // Save in case make_unique calls malloc
    // Allocate this on the heap so we don't bloat the stack in the event of success
    auto buffer = std::make_unique<char[]>(1024);
    strerror_r(err, buffer.get(), 1024);
    throw socket_exception{buffer.get()};
  }
  r = ::listen(d_descriptor6, backlog);
  if (r) {
    auto err = errno;
    auto buffer = std::make_unique<char[]>(1024);
    strerror_r(err, buffer.get(), 1024);
    throw socket_exception{buffer.get()};
  }
}

std::unique_ptr<addrinfo, addrinfo_del> get_addr_info(const std::string &name,
                                                      const std::string &service,
                                                      const addrinfo &hints) {
  addrinfo result = {};
  auto result_p = &result;

  const char *name_cs = nullptr;
  if (!name.empty()) name_cs = name.c_str();
  const char *service_cs = nullptr;
  if (!service.empty()) service_cs = service.c_str();

  auto rcode = getaddrinfo(name_cs, service_cs, &hints, &result_p);
  auto ptr = std::unique_ptr<addrinfo, addrinfo_del>(result_p);
  if (rcode) throw socket_exception{gai_strerror(rcode)};

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
        sockaddr_in *ipv4 = reinterpret_cast<sockaddr_in *>(p->ai_addr);
        addr = &(ipv4->sin_addr);
        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
        v4.emplace_back(ipstr);
        break;
      }
      case AF_INET6: {  // IPv6
        char ipstr[INET6_ADDRSTRLEN];
        sockaddr_in6 *ipv6 = reinterpret_cast<sockaddr_in6 *>(p->ai_addr);
        addr = &(ipv6->sin6_addr);
        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
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
