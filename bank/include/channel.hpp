#ifndef INCLUDE_CHANNEL_HPP_
#define INCLUDE_CHANNEL_HPP_
#include "message.hpp"
#include "proto/bank.pb.h"
#include <boost/asio.hpp>
#include <deque>
#include <memory>
#include <string>

class Channel : public std::enable_shared_from_this<Channel> {
 public:
  Channel(boost::asio::ip::tcp::socket socket, boost::asio::io_service &io_service,
          std::function<void(const BranchMessage &)> msg_handler);

  Channel(boost::asio::io_service &io_service,
          std::function<void(const BranchMessage &)> msg_handler);

  ~Channel() = default;

  Channel(const Channel &) = delete;
  Channel(Channel &&) = delete;
  Channel &operator=(const Channel &) = delete;
  Channel &operator=(Channel &&) = delete;

  void send_branch_msg(const BranchMessage &msg);

  void start();

  void connect(const boost::asio::ip::tcp::resolver::iterator &endpoint);

  template <typename Handler>
  void connect_cb(const boost::asio::ip::tcp::resolver::iterator &endpoint, Handler &&handler) {
    std::cout << "Connecting..." << std::endl;
    boost::asio::async_connect(
        d_socket, endpoint,
        [this, handler](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator) {
          if (!ec) {
            d_socket.get_io_service().post(handler);
            start();
          } else {
            throw std::runtime_error{"Unable to connect to peer: " + ec.message()};
          }
        });
  }

  auto &handler() { return d_msg_handler; }

 protected:
  void serve_request();

  void handle_message(const BranchMessage &msg);

  void do_send();

  void do_read_header();

  void do_read_message();

 private:
  boost::asio::ip::tcp::socket d_socket;
  std::deque<std::pair<std::shared_ptr<char[]>, size_t>> d_mq;
  BranchNetMsg d_msg;
  std::function<void(const BranchMessage &)> d_msg_handler;
  boost::asio::strand d_strand;
};
#endif
