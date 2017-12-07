#pragma once
#include "channel.hpp"
#include "config.hpp"
#include "log.hpp"
#include "server.pb.h"
#include <array>
#include <boost/asio.hpp>
#include <chrono>
#include <memory>
#include <string>

namespace kvstore::server {
class Replica : public std::enable_shared_from_this<Replica> {
 public:
  struct storage_element {
    storage_element(std::string adata, ::google::protobuf::Timestamp stamp)
        : data{std::move(adata)}, timestamp{std::move(stamp)} {}
    std::string data;
    ::google::protobuf::Timestamp timestamp;
  };

  Replica(boost::asio::io_service &io_svc, Config cfg);

  std::shared_ptr<Replica::storage_element> read(uint32_t key);

  /**
   * Writes data at key if the timestamp is after the currently held data
   * Uses atomic compare and exchange to provide thread safety.
   */
  void write(uint32_t key, const std::string &data, const google::protobuf::Timestamp &timestamp);

 private:
  void replay_handler(const DoWrite &msg);

  /* Key to Value map */
  boost::asio::io_service &m_io_service;
  std::array<std::shared_ptr<storage_element>, 256> m_store;
  Config m_config;
  std::shared_ptr<Log<DoWrite>> m_log;
  std::vector<std::shared_ptr<Channel<ServerMessage>>> m_channels;
};

}  // namespace kvstore::server
