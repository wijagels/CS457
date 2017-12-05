#pragma once
#include "message.hpp"
#include "server.pb.h"
#if __has_include(<filesystem>)
#include <filesystem>
#else
#include <experimental/filesystem>
#endif
#include <boost/asio.hpp>
#include <fstream>

namespace kvstore::server {
#if __has_include(<filesystem>)
namespace stdfs = std::filesystem;
#else
namespace stdfs = std::experimental::filesystem;
#endif

template <typename M>
class Log : public std::enable_shared_from_this<Log<M>> {
  using std::enable_shared_from_this<Log<M>>::shared_from_this;

 public:
  Log(stdfs::path file_path, boost::asio::io_service &io_service)
      : m_file_path{std::move(file_path)},
        m_file{m_file_path, std::ios::out | std::ios::binary | std::ios::ate | std::ios::app},
        m_io_service{io_service},
        m_strand{m_io_service} {}

  /**
   * Clears the contents of the log
   */
  void clear() {
    m_file.close();
    m_file.open(m_file_path, std::ios::out | std::ios::binary | std::ios::ate | std::ios::trunc);
  }

  /**
   * Asynchronously update the log with a server message.
   * Flushes before calling the completion handler.
   */
  template <typename Handler>
  void write_update(const M &msg, Handler &&handler) {
    auto self = shared_from_this();
    m_strand.post([this, self, msg, handler]() {
      messaging::Message encoded;
      encoded.set_body_size(msg.ByteSizeLong());
      m_file.write(reinterpret_cast<char *>(encoded.header().data()), encoded.header_length);
      msg.SerializeToOstream(&m_file);
      m_file.flush();
      handler();
    });
  }

  template <typename Handler>
  void replay_log(Handler &&handler) {
    auto self = shared_from_this();
    m_strand.post([this, self, handler]() {
      auto file = std::ifstream{m_file_path, std::ios::in | std::ios::binary};
      messaging::Message msg;
      auto read_buf = reinterpret_cast<char *>(msg.header().data());
      std::size_t alloc_size = 32;
      auto temp_buf = std::make_unique<char[]>(alloc_size);
      while (file.good()) {
        file.read(read_buf, msg.header_length);
        if (file.eof()) break;
        msg.decode_header();
        if (msg.body_size() > alloc_size) {
          alloc_size = msg.body_size();
          temp_buf = std::make_unique<char[]>(alloc_size);
        }
        file.read(temp_buf.get(), msg.body_size());
        M message;
        message.ParseFromArray(temp_buf.get(), msg.body_size());
        handler(std::move(message));
      }
    });
  }

 private:
  // Implicitly move-only because of fstream
  const stdfs::path m_file_path;
  std::fstream m_file;
  boost::asio::io_service &m_io_service;
  boost::asio::strand m_strand;
};
}  // namespace kvstore::server
