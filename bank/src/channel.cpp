#include "channel.hpp"
#include "common.hpp"
#include "message.hpp"
#include "proto/bank.pb.h"
#include <memory>

Channel::Channel(boost::asio::ip::tcp::socket socket, std::shared_ptr<Branch> branch)
    : d_socket{std::move(socket)}, d_branch{std::move(branch)} {}

void Channel::send_branch_msg(const BranchMessage &msg) {
  bool empty = d_mq.empty();
  auto size = msg.ByteSizeLong();
  std::shared_ptr buf = std::make_unique<char[]>(size);
  msg.SerializeToArray(buf.get(), size);
  d_mq.emplace_back(buf, size);
  if (empty) {
    do_send();
  }
}

void Channel::start() { do_read_header(); }

void Channel::connect(const boost::asio::ip::tcp::resolver::iterator &endpoint) {
  boost::asio::async_connect(
      d_socket, endpoint,
      [this](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator) {
        if (!ec) {
          start();
        } else {
          throw std::runtime_error{"Unable to connect to peer: " + ec.message()};
        }
      });
}

void Channel::do_send() {
  auto self = shared_from_this();
  auto[buf, size] = d_mq.front();
  auto msg = std::make_shared<BranchNetMsg>();
  msg->body_size() = size;
  msg->encode_header();
  std::array<boost::asio::const_buffer, 2> buf_seq{
      {boost::asio::buffer(msg->header()), boost::asio::const_buffer{buf.get(), size}}};
  auto handler = [this, self, msg](boost::system::error_code ec, size_t) {
    if (!ec) {
      d_mq.pop_front();
      if (!d_mq.empty()) {
        do_send();
      }
    }
  };
  boost::asio::async_write(d_socket, std::move(buf_seq), std::move(handler));
}

void Channel::do_read_header() {
  auto self = shared_from_this();
  auto buf = boost::asio::buffer(d_msg.header());
  boost::asio::async_read(d_socket, buf, [this, self](boost::system::error_code ec, size_t) {
    if (!ec) {
      std::cout << "Got header" << std::endl;
      d_msg.decode_header();
      std::cout << d_msg.body_size() << std::endl;
    }
  });
}

void Channel::do_read_message() {
  auto size = d_msg.body_size();
  std::cout << size << std::endl;
  std::shared_ptr buf = std::make_unique<char[]>(size);
  auto self = shared_from_this();
  boost::asio::async_read(d_socket, boost::asio::buffer(buf.get(), size),
                          [this, self, buf, size](boost::system::error_code ec, size_t) {
                            if (!ec) {
                              BranchMessage msg;
                              msg.ParseFromArray(buf.get(), size);
                              d_branch->handle_message(msg);
                              do_read_header();
                            }
                          });
}
