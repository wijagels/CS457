#include "controller.hpp"
#include <memory>
#include <boost/asio.hpp>

int main(int argc, char **argv) {
  if (argc != 3) {
    throw std::invalid_argument{"Invalid invocation"};
  }
  boost::asio::io_service io_service;
  auto controller = std::make_shared<Controller>(std::stoull(argv[1]), io_service);
  controller->start(argv[2]);
  io_service.run();
}
