#include "branch.hpp"
#include "netutils.hpp"
#include <memory>
#include <thread>

int main(int argc, char *argv[]) {
  if (argc != 3) return 1;
  std::string name{argv[1]};
  uint16_t port = std::atoi(argv[2]);
  auto addr = boost::asio::ip::address::from_string(get_public_ip());
  auto endpoint = boost::asio::ip::tcp::endpoint{addr, port};
  boost::asio::io_service io_service;
  std::shared_ptr<Branch> branch = std::make_shared<Branch>(name, endpoint, io_service);
  branch->start();
  // auto worker = [&io_service]() {
  //   try {
      io_service.run();
    // } catch (const std::runtime_error &ex) {
    //   std::cerr << ex.what() << std::endl;
    //   std::exit(0);
    // }
  // };
  // std::thread t1{worker};
  // std::thread t2{worker};
  // std::thread t3{worker};
  // std::thread t4{worker};
  // t1.join();
  // t2.join();
  // t3.join();
  // t4.join();
}
