#include "gtest/gtest.h"
#include "socket.hpp"
#include "mimedb.hpp"
#include <fstream>

extern "C" {
#include <arpa/inet.h>
}

TEST(SocketTest, gethostbyname) { // NOLINT
  get_host_by_name("localhost");
}

TEST(SocketTest, getaddrinfo) {  // NOLINT
  auto x = get_addr_info("localhost", "8080");
  auto ips = addr_vec_from_addrinfo(*x);
  EXPECT_EQ("127.0.0.1", ips.first.front());
}

TEST(SocketTest, getaddrinfo_any) {  // NOLINT
  addrinfo hint = {};
  hint.ai_family = AF_UNSPEC;
  hint.ai_flags = AI_PASSIVE;  // Grabs the any address
  auto x = get_addr_info("", "8080", hint);
  EXPECT_TRUE(x.get());
  auto ips = addr_vec_from_addrinfo(*x);
  EXPECT_EQ("0.0.0.0", ips.first.front());
}

TEST(SocketTest, streamsockconn) {  // NOLINT
  const std::string req =
      "GET /headers HTTP/1.1\r\n"
      "Host: wtfismyip.com\r\n"
      "User-Agent: devel/1.0\r\n"
      "Accept: text/plain\r\n"
      "\r\n";

  StreamSocket g{"wtfismyip.com", 80};
  g.send(req);
  std::cout << g.recv() << std::endl;
}

TEST(MimeDbTest, html) {  // NOLINT
  MimeDb mdb;
  EXPECT_EQ(mdb.mime_of_ext("html"), "text/html");
}
