#include "controller.hpp"
#include <boost/asio.hpp>
#include <memory>

int main(int argc, char **argv) {
  if (argc != 3) {
    throw std::invalid_argument{"Invalid invocation"};
  }
  boost::asio::io_service io_service;
  auto controller = std::make_shared<Controller>(std::stoull(argv[1]), io_service);
  controller->start(argv[2]);
  try {
    io_service.run();
  } catch (const std::runtime_error &ex) {
    std::cerr << ex.what() << std::endl;
    std::exit(0);
  }
}
