#include "client.hpp"
#include "channel.hpp"
#include "client.pb.h"
#include <boost/asio.hpp>
#include <iostream>
#include <regex>

namespace kvstore::client {
Client::Client(boost::asio::io_service &io_service)
    : m_io_service{io_service},
      m_channel{std::make_shared<Channel<ClientMessage>>(
          m_io_service,
          [](const ClientMessage &, const std::shared_ptr<Channel<ClientMessage>> &) {})},
      m_timer{m_io_service} {}

void Client::connect(const std::string &host, const std::string &port) {
  using boost::asio::ip::tcp;
  auto self = shared_from_this();
  m_channel->handler() = [this, self](const ClientMessage &msg,
                                      const std::shared_ptr<Channel<ClientMessage>> &) {
    message_handler(msg);
  };
  tcp::resolver resolver{m_io_service};
  m_channel->connect_cb(resolver.resolve({host, port}), [this, self]() { run_cmd(); });
}

void Client::message_handler(const ClientMessage &msg) {
  switch (msg.msg_case()) {
    case ClientMessage::kPutKeyResp: {
      auto resp = msg.put_key_resp();
      std::cout << "Put @" << resp.key() << '\n';
      break;
    }
    case ClientMessage::kGetKeyResp: {
      auto resp = msg.get_key_resp();
      std::cout << "Get @" << resp.key() << " = " << resp.val() << '\n';
      break;
    }
    default:
      break;
  }
  run_cmd();
}

void Client::run_cmd() {
  m_timer.cancel();
  std::string command;
  std::cout << "Command> ";
  std::getline(std::cin, command);
  if (std::cin.eof()) exit(0);
  const std::regex get_re{R"(^get (\d) (\d+)$)", std::regex::icase};
  const std::regex put_re{R"(^put (\d) (\d+) (.+)$)", std::regex::icase};
  std::smatch match;
  auto self = shared_from_this();
  if (std::regex_match(command, match, get_re)) {
    ClientMessage cm;
    cm.mutable_get_key()->set_consistency(std::stoi(match[1].str()));
    cm.mutable_get_key()->set_key(std::stoi(match[2].str()));
    cm.mutable_get_key()->set_stream(0);
    m_channel->send_msg(cm);
    m_timer.expires_from_now(boost::posix_time::seconds(5));
    m_timer.async_wait([this, self](boost::system::error_code ec) {
      if (!ec) {
        std::cout << "Last command failed\n";
        run_cmd();
      }
    });
  } else if (std::regex_match(command, match, put_re)) {
    ClientMessage cm;
    cm.mutable_put_key()->set_consistency(std::stoi(match[1].str()));
    cm.mutable_put_key()->set_key(std::stoi(match[2].str()));
    cm.mutable_put_key()->set_val(match[3].str());
    cm.mutable_put_key()->set_stream(0);
    m_channel->send_msg(cm);
    m_timer.expires_from_now(boost::posix_time::seconds(5));
    m_timer.async_wait([this, self](boost::system::error_code ec) {
      if (!ec) {
        std::cout << "Last command failed\n";
        run_cmd();
      }
    });
  } else {
    std::cout << "Invalid command\n";
    run_cmd();
  }
}
}  // namespace kvstore::client

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Invalid number of args\n";
    exit(1);
  }
  boost::asio::io_service io_service;
  auto c = std::make_shared<kvstore::client::Client>(io_service);
  c->connect(argv[1], argv[2]);
  io_service.run();
}
