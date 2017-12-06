#include "coordinator.cpp"
#include <iostream>
#include <boost/asio.hpp>

using namespace kvstore::server;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Invalid number of arguments\n";
    exit(1);
  }
  boost::asio::io_service io_service;
  Config cfg{argv[1]};
  Coordinator c{io_service, cfg};
  c.start();
  io_service.run();
}
