#pragma once
#include "channel.hpp"
#include "client.pb.h"
#include "config.hpp"
#include "replica.hpp"
#include "server.pb.h"
#include <boost/asio.hpp>
#include <boost/uuid/random_generator.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

namespace kvstore::server {
class Coordinator : public std::enable_shared_from_this<Coordinator> {
 public:
  Coordinator(boost::asio::io_service &io_service, Config cfg);
  void do_read(const DoRead &msg);

 protected:
  void handle_read_repair_response(const ReadRepairResponse &msg,
                                   std::shared_ptr<Channel<ServerMessage>> from);

  void handle_get(const client::GetKey &msg, std::shared_ptr<Channel<client::ClientMessage>> from);

 private:
  struct pending_read {
    pending_read(uint32_t astream, uint32_t akey, uint32_t aconsistency,
                 std::shared_ptr<Channel<client::ClientMessage>> arequestor,
                 std::vector<std::pair<::google::protobuf::Timestamp, std::string>> aresults)
        : stream{astream},
          key{akey},
          consistency{aconsistency},
          requestor{std::move(arequestor)},
          results{std::move(aresults)} {}
    uint32_t stream;
    uint32_t key;
    uint32_t consistency;
    std::shared_ptr<Channel<client::ClientMessage>> requestor;
    std::vector<std::pair<::google::protobuf::Timestamp, std::string>> results;
  };
  struct pending_write {
    pending_write(uint32_t astream, uint32_t akey, uint32_t aconsistency,
                  std::shared_ptr<Channel<client::ClientMessage>> arequestor)
        : stream{astream}, key{akey}, consistency{aconsistency}, requestor{std::move(arequestor)} {}
    uint32_t stream;
    uint32_t key;
    uint32_t consistency;
    std::shared_ptr<Channel<client::ClientMessage>> requestor;
  };
  boost::asio::io_service &m_io_service;
  Config m_config;
  Replica m_replica;
  std::vector<std::shared_ptr<Channel<ServerMessage>>> m_channels;

  std::unordered_map<std::string, pending_read> m_pending_reads;
  boost::asio::strand m_pending_read_strand{m_io_service};

  std::unordered_map<std::string, pending_write> m_pending_writes;
  boost::asio::strand m_pending_write_strand{m_io_service};

  boost::uuids::random_generator m_uuid_gen{};
};
}  // namespace kvstore::server
