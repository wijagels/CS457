#pragma once
#include "message.hpp"
#include <boost/asio.hpp>
#include <type_traits>
#include <memory>
#include <deque>
#include <google/protobuf/message.h>

namespace kvstore {
template <typename M, typename = std::enable_if_t<std::is_base_of_v<google::protobuf::Message, M>>>
class Channel : public std::enable_shared_from_this<Channel<M>> {
  using std::enable_shared_from_this<Channel<M>>::shared_from_this;

 public:
  Channel(boost::asio::ip::tcp::socket &&socket, boost::asio::io_service &io_service,
          std::function<void(const M &, const std::shared_ptr<Channel<M>> &)> msg_handler)
      : m_io_service{io_service},
        m_socket{std::move(socket)},
        m_strand{m_io_service},
        m_msg_handler{std::move(msg_handler)},
        m_active{false},
        m_reconnect_timer{m_io_service} {}

  Channel(boost::asio::io_service &io_service,
          std::function<void(const M &, const std::shared_ptr<Channel<M>> &)> msg_handler)
      : m_io_service{io_service},
        m_socket{m_io_service},
        m_strand{m_io_service},
        m_msg_handler{std::move(msg_handler)},
        m_active{true},
        m_reconnect_timer{m_io_service} {}

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
    m_peer = endpoint;
    boost::asio::async_connect(
        m_socket, endpoint,
        [this, self](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator) {
          if (!ec) {
            start();
          } else {
            std::cerr << "Unable to connect to peer: " + ec.message() << '\n';
            reconnect();
          }
        });
  }

  template <typename Handler>
  void connect_cb(const boost::asio::ip::tcp::resolver::iterator &endpoint, Handler &&handler) {
    auto self = shared_from_this();
    m_peer = endpoint;
    boost::asio::async_connect(m_socket, endpoint,
                               [this, self, handler](boost::system::error_code ec,
                                                     boost::asio::ip::tcp::resolver::iterator) {
                                 if (!ec) {
                                   m_socket.get_io_service().post(handler);
                                   start();
                                 } else {
                                   std::cerr << "Unable to connect to peer: " + ec.message()
                                             << '\n';
                                   reconnect();
                                 }
                               });
  }

  auto &handler() noexcept { return m_msg_handler; }

  auto &reconnect_handler() noexcept { return m_reconnect_handler; }

 protected:
  void reconnect() {
    if (!m_active) return;
    auto self = shared_from_this();
    m_reconnect_timer.expires_from_now(boost::posix_time::seconds(5));
    auto handler = m_strand.wrap([this, self](boost::system::error_code ec) {
      if (!ec) {
        m_mq.clear();
        connect_cb(m_peer, [this, self]() { m_reconnect_handler(self); });
      } else {
        std::cerr << ec.message() << '\n';
      }
    });
    m_reconnect_timer.async_wait(handler);
  }

  void do_send() {
    auto self = shared_from_this();
    auto[buf, size] = m_mq.front();
    messaging::Message msg;
    msg.set_body_size(size);
    std::array<boost::asio::const_buffer, 2> buf_seq{
        {boost::asio::buffer(msg.header()), boost::asio::const_buffer{buf.get(), size}}};
    auto handler = m_strand.wrap([this, self](boost::system::error_code ec, size_t) {
      if (!ec) {
        m_mq.pop_front();
        if (!m_mq.empty()) {
          do_send();
        }
      } else {
        std::cerr << "Sending failed: " + ec.message() << '\n';
        reconnect();
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
        std::cerr << "Reading header failed: " + ec.message() << '\n';
        reconnect();
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
                                m_msg_handler(msg, self);
                                do_read_header();
                              } else {
                                std::cerr << "Reading message failed: " + ec.message() << '\n';
                                reconnect();
                              }
                            });
  }

 private:
  boost::asio::io_service &m_io_service;
  boost::asio::ip::tcp::socket m_socket;
  std::deque<std::pair<std::shared_ptr<char[]>, size_t>> m_mq;
  messaging::Message m_msg;
  boost::asio::strand m_strand;
  std::function<void(const M &, const std::shared_ptr<Channel<M>> &)> m_msg_handler;
  std::function<void(const std::shared_ptr<Channel<M>> &)> m_reconnect_handler =
      [](const std::shared_ptr<Channel<M>> &) {};
  bool m_active;
  boost::asio::ip::tcp::resolver::iterator m_peer;
  boost::asio::deadline_timer m_reconnect_timer;
};
}  // namespace kvstore
