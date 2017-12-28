#pragma once
#include "channel.hpp"
#include "client.pb.h"
#include <boost/asio.hpp>
#include <memory>

int main(int argc, char *argv[]);

namespace kvstore::client {
class Client : public std::enable_shared_from_this<Client> {
 public:
  Client(boost::asio::io_service &io_service);

  void connect(const std::string &host, const std::string &port);

 private:
  void message_handler(const ClientMessage &msg);

  void run_cmd();

  boost::asio::io_service &m_io_service;
  std::shared_ptr<Channel<ClientMessage>> m_channel;
  boost::asio::deadline_timer m_timer;
};
}  // namespace kvstore::client
