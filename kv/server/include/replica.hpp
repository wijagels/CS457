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
    std::string data;
    google::protobuf::Timestamp timestamp;
  };

  Replica(boost::asio::io_service &io_svc, Config cfg);

  std::shared_ptr<Replica::storage_element> read(uint32_t key);

  void write(uint32_t key, std::string data, google::protobuf::Timestamp timestamp);

 private:
  /* Key to Value map */
  boost::asio::io_service &m_io_service;
  std::array<std::shared_ptr<storage_element>, 256> m_store;
  Config m_config;
  Log m_log;
  std::vector<std::shared_ptr<Channel<ServerMessage>>> m_channels;
};

}  // namespace kvstore::server
