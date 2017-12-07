#include "coordinator.cpp"
#include <boost/asio.hpp>
#include <iostream>

using namespace kvstore::server;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Invalid number of arguments\n";
    exit(1);
  }
  boost::asio::io_service io_service;
  Config cfg{argv[1]};
  auto c = std::make_shared<Coordinator>(io_service, cfg);
  c->start();
  io_service.run();
}
