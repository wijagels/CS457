#include "socket.hpp"

static constexpr uint16_t portnum = 8081;

int main() {
  Socket socket;
  socket.bind_address("localhost", portnum);
  socket.listen(100);
}
