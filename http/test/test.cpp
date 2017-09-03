#include "gtest/gtest.h"
#include "socket.hpp"

extern "C" {
#include <arpa/inet.h>
}

TEST(SocketTest, gethostbyname) { get_host_by_name("localhost"); }

TEST(SocketTest, getaddrinfo) {
  auto x = get_addr_info("localhost", "8080");
  auto ips = addr_vec_from_addrinfo(*x);
  EXPECT_EQ("127.0.0.1", ips.first.front());
}

TEST(SocketTest, getaddrinfo_any) {
  addrinfo hint = {};
  hint.ai_family = AF_UNSPEC;
  hint.ai_flags = AI_PASSIVE;  // Grabs the any address
  auto x = get_addr_info("", "8080", hint);
  EXPECT_TRUE(x.get());
  auto ips = addr_vec_from_addrinfo(*x);
  EXPECT_EQ("0.0.0.0", ips.first.front());
}
