#include "channel.hpp"
#include "common.hpp"
#include "message.hpp"
#include "proto/bank.pb.h"
#include <memory>

Channel::Channel(boost::asio::ip::tcp::socket &&socket, boost::asio::io_service &io_service,
                 std::function<void(const BranchMessage &)> msg_handler)
    : d_socket{std::move(socket)}, d_msg_handler{std::move(msg_handler)}, d_strand{io_service} {}

Channel::Channel(boost::asio::io_service &io_service,
                 std::function<void(const BranchMessage &)> msg_handler)
    : d_socket{io_service}, d_msg_handler{std::move(msg_handler)}, d_strand{io_service} {}

void Channel::send_branch_msg(const BranchMessage &msg) {
  auto self = shared_from_this();
  d_strand.post([this, self, msg]() {
    bool empty = d_mq.empty();
    auto size = msg.ByteSizeLong();
    std::shared_ptr buf = std::make_unique<char[]>(size);
    msg.SerializeToArray(buf.get(), size);
    d_mq.emplace_back(buf, size);
    if (empty) {
      do_send();
    }
  });
}

void Channel::start() { do_read_header(); }

void Channel::connect(const boost::asio::ip::tcp::resolver::iterator &endpoint) {
  auto self = shared_from_this();
  boost::asio::async_connect(
      d_socket, endpoint,
      [this, self](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator) {
        if (!ec) {
          start();
        } else {
          std::cerr << "Unable to connect to peer: " << ec.message() << std::endl;
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
  auto handler = d_strand.wrap([this, self, msg](boost::system::error_code ec, size_t) {
    if (!ec) {
      d_mq.pop_front();
      if (!d_mq.empty()) {
        do_send();
      }
    } else {
      throw std::runtime_error{"Sending failed: " + ec.message()};
    }
  });
  boost::asio::async_write(d_socket, std::move(buf_seq), std::move(handler));
}

void Channel::do_read_header() {
  auto self = shared_from_this();
  auto buf = boost::asio::buffer(d_msg.header());
  boost::asio::async_read(d_socket, buf, [this, self](boost::system::error_code ec, size_t) {
    if (!ec) {
      d_msg.decode_header();
      do_read_message();
    } else {
      throw std::runtime_error{"Reading header failed: " + ec.message()};
    }
  });
}

void Channel::do_read_message() {
  auto size = d_msg.body_size();
  std::shared_ptr buf = std::make_unique<char[]>(size);
  auto self = shared_from_this();
  boost::asio::async_read(d_socket, boost::asio::buffer(buf.get(), size),
                          [this, self, buf, size](boost::system::error_code ec, size_t) {
                            if (!ec) {
                              BranchMessage msg;
                              msg.ParseFromArray(buf.get(), size);
                              d_msg_handler(msg);
                              do_read_header();
                            } else {
                              throw std::runtime_error{"Reading message failed: " + ec.message()};
                            }
                          });
}
