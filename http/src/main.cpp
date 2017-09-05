#include "socket.hpp"
#include <iostream>

static constexpr uint16_t g_portnum = 8081;

int main() {
  StreamServerSocket socket{g_portnum};
  socket.listen(10);
  std::cout << socket.accept().recv() << std::endl;
}
