#ifndef INCLUDE_CHANNEL_HPP_
#define INCLUDE_CHANNEL_HPP_
#include "proto/bank.pb.h"
#include "message.hpp"
#include "branch.hpp"
#include <boost/asio.hpp>
#include <deque>
#include <memory>
#include <string>

class Branch;

class Channel : public std::enable_shared_from_this<Channel> {
 public:
  Channel(boost::asio::ip::tcp::socket socket, std::shared_ptr<Branch> branch);

  ~Channel() = default;

  Channel(const Channel &) = delete;
  Channel(Channel &&) = delete;
  Channel &operator=(const Channel &) = delete;
  Channel &operator=(Channel &&) = delete;

  void send_branch_msg(const BranchMessage &msg);

  void start();

  void connect(const boost::asio::ip::tcp::resolver::iterator &endpoint);

  auto peer() {
    return d_socket.remote_endpoint();
  }

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
  std::shared_ptr<Branch> d_branch;
};
#endif
