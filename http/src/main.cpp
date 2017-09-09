#include "server.hpp"
#include "mimedb.hpp"
#include "http.hpp"
#include <csignal>
#include <cstdint>
#include <future>
#include <iostream>

static constexpr uint16_t g_portnum = 8081;

void sig_handler(int s);

void sig_handler(int s) { throw s; }

int main() {
  MimeDb mdb{"/etc/mime.types"};
  std::cout << http_time() << std::endl;
  std::signal(SIGINT, sig_handler);  // Graceful ctrl-c shutdown
  try {
    HttpServer server{g_portnum};
    server.serve_get("/../../../index.html");
    server.listen(10);
    server.accept_connections();
  } catch (int) {
    return 1;
  }
}
