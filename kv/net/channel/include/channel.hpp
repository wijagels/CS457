#pragma once
#include "message.hpp"
#include <boost/asio.hpp>
#include <deque>
#include <google/protobuf/message.h>

namespace kvstore {
template <typename M, typename = std::enable_if_t<std::is_base_of_v<google::protobuf::Message, M>>>
class Channel : public std::enable_shared_from_this<Channel<M>> {
  using std::enable_shared_from_this<Channel<M>>::shared_from_this;

 public:
  Channel(boost::asio::ip::tcp::socket &&socket, boost::asio::io_service &io_service,
          std::function<void(const M &)> msg_handler)
      : m_socket{std::move(socket)}, m_strand{io_service}, m_msg_handler{std::move(msg_handler)} {}

  Channel(boost::asio::io_service &io_service, std::function<void(const M &)> msg_handler)
      : m_socket{io_service}, m_strand{io_service}, m_msg_handler{std::move(msg_handler)} {}

  ~Channel() = default;

  Channel(const Channel &) = delete;
  Channel(Channel &&) = delete;
  Channel &operator=(const Channel &) = delete;
  Channel &operator=(Channel &&) = delete;

  void send_msg(const M &msg) {
    auto self = shared_from_this();
    m_strand.post([this, self, msg]() {
      bool empty = m_mq.empty();
      auto size = msg.ByteSizeLong();
      std::shared_ptr buf = std::make_unique<char[]>(size);
      msg.SerializeToArray(buf.get(), size);
      m_mq.emplace_back(buf, size);
      if (empty) {
        do_send();
      }
    });
  }

  void start() { do_read_header(); }

  void connect(const boost::asio::ip::tcp::resolver::iterator &endpoint) {
    auto self = shared_from_this();
    boost::asio::async_connect(
        m_socket, endpoint,
        [this, self](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator) {
          if (!ec) {
            start();
          } else {
            throw std::runtime_error{"Unable to connect to peer: " + ec.message()};
          }
        });
  }

  template <typename Handler>
  void connect_cb(const boost::asio::ip::tcp::resolver::iterator &endpoint, Handler &&handler) {
    auto self = shared_from_this();
    boost::asio::async_connect(
        m_socket, endpoint,
        [this, self, handler](boost::system::error_code ec,
                              boost::asio::ip::tcp::resolver::iterator) {
          if (!ec) {
            m_socket.get_io_service().post(handler);
            start();
          } else {
            throw std::runtime_error{"Unable to connect to peer: " + ec.message()};
          }
        });
  }

  auto &handler() noexcept { return m_msg_handler; }

 protected:
  void do_send() {
    auto self = shared_from_this();
    auto[buf, size] = m_mq.front();
    messaging::Message msg;
    msg.set_body_size(size);
    std::array<boost::asio::const_buffer, 2> buf_seq{
        {boost::asio::buffer(msg.header()), boost::asio::const_buffer{buf.get(), size}}};
    auto handler = m_strand.wrap([this, self, msg](boost::system::error_code ec, size_t) {
      if (!ec) {
        m_mq.pop_front();
        if (!m_mq.empty()) {
          do_send();
        }
      } else {
        throw std::runtime_error{"Sending failed: " + ec.message()};
      }
    });
    boost::asio::async_write(m_socket, std::move(buf_seq), std::move(handler));
  }

  void do_read_header() {
    auto self = shared_from_this();
    auto buf = boost::asio::buffer(m_msg.header());
    boost::asio::async_read(m_socket, buf, [this, self](boost::system::error_code ec, size_t) {
      if (!ec) {
        m_msg.decode_header();
        do_read_message();
      } else {
        throw std::runtime_error{"Reading header failed: " + ec.message()};
      }
    });
  }

  void do_read_message() {
    auto size = m_msg.body_size();
    std::shared_ptr buf = std::make_unique<char[]>(size);
    auto self = shared_from_this();
    boost::asio::async_read(m_socket, boost::asio::buffer(buf.get(), size),
                            [this, self, buf, size](boost::system::error_code ec, size_t) {
                              if (!ec) {
                                M msg;
                                msg.ParseFromArray(buf.get(), size);
                                m_msg_handler(msg);
                                do_read_header();
                              } else {
                                throw std::runtime_error{"Reading message failed: " + ec.message()};
                              }
                            });
  }

 private:
  boost::asio::ip::tcp::socket m_socket;
  std::deque<std::pair<std::shared_ptr<char[]>, size_t>> m_mq;
  messaging::Message m_msg;
  boost::asio::strand m_strand;
  std::function<void(const M &, std::shared_ptr<Channel>)> m_msg_handler;
};
}  // namespace kvstore
