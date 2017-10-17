#include "channel.hpp"
#include "server.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
  printf("%s, %d\n", argv[0], argc);
  server srv{"tcp://*:5555"};

}
