#include "netutils.hpp"
#include "branch.hpp"
#include <thread>
#include <memory>

int main(int argc, char *argv[]) {
  if (argc != 3) return 1;
  std::string name{argv[1]};
  uint16_t port = std::atoi(argv[2]);
  auto addr = boost::asio::ip::address::from_string(get_public_ip());
  auto endpoint = boost::asio::ip::tcp::endpoint{addr, port};
  boost::asio::io_service io_service;
  std::shared_ptr<Branch> branch = std::make_shared<Branch>(name, endpoint, io_service);
  branch->start();
  io_service.run();
}
