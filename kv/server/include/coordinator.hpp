#pragma once
#include "channel.hpp"
#include "client.pb.h"
#include "config.hpp"
#include "hint.pb.h"
#include "replica.hpp"
#include "server.pb.h"
#include <boost/asio.hpp>
#include <boost/uuid/random_generator.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

namespace kvstore::server {
class Coordinator : public std::enable_shared_from_this<Coordinator> {
  struct pending_read {
    pending_read(uint32_t astream, uint32_t akey, uint32_t aconsistency,
                 std::shared_ptr<Channel<client::ClientMessage>> arequestor,
                 std::vector<std::pair<::google::protobuf::Timestamp, std::string>> aresults)
        : stream{astream},
          key{akey},
          consistency{aconsistency},
          requestor{std::move(arequestor)},
          results{std::move(aresults)} {}
    const uint32_t stream;
    const uint32_t key;
    uint32_t consistency;
    std::shared_ptr<Channel<client::ClientMessage>> requestor;
    std::vector<std::pair<::google::protobuf::Timestamp, std::string>> results;
  };
  struct pending_write {
    pending_write(uint32_t astream, uint32_t akey, uint32_t aconsistency,
                  std::shared_ptr<Channel<client::ClientMessage>> arequestor)
        : stream{astream}, key{akey}, consistency{aconsistency}, requestor{std::move(arequestor)} {}
    const uint32_t stream;
    const uint32_t key;
    uint32_t consistency;
    const std::shared_ptr<Channel<client::ClientMessage>> requestor;
  };
  struct hint {
    uint32_t key;
    ::google::protobuf::Timestamp timestamp;
  };

 public:
  Coordinator(boost::asio::io_service &io_service, Config cfg);

  void start();

 protected:
  void do_accept();

  void do_client_accept();

  void handle_read_repair_response(const ReadRepairResponse &msg,
                                   const std::shared_ptr<Channel<ServerMessage>> &from);

  void handle_read_repair(const ReadRepair &msg,
                          const std::shared_ptr<Channel<ServerMessage>> &from);

  void handle_get(const client::GetKey &msg,
                  const std::shared_ptr<Channel<client::ClientMessage>> &from);

  void handle_put(const client::PutKey &msg,
                  const std::shared_ptr<Channel<client::ClientMessage>> &from);

  void handle_write_complete(const WriteComplete &msg, uint32_t from);

  void handle_do_write(const DoWrite &msg, const std::shared_ptr<Channel<ServerMessage>> &ptr,
                       uint32_t from);

  void handle_do_read(const DoRead &msg, const std::shared_ptr<Channel<ServerMessage>> &ptr,
                      uint32_t from);

  void handle_read_complete(const ReadComplete &msg,
                            const std::shared_ptr<Channel<ServerMessage>> &ptr, uint32_t from);

  void unintroduced_msg_handler(const ServerMessage &msg,
                                const std::shared_ptr<Channel<ServerMessage>> &ptr);

  void server_msg_handler(const ServerMessage &msg,
                          const std::shared_ptr<Channel<ServerMessage>> &ptr, uint32_t id);

  void client_handler(const client::ClientMessage &msg,
                      const std::shared_ptr<Channel<client::ClientMessage>> &ptr);

  void initialize_outgoing();

 private:
  boost::asio::io_service &m_io_service;
  Config m_config;
  std::shared_ptr<Replica> m_replica;
  std::vector<std::shared_ptr<Channel<ServerMessage>>> m_channels;
  std::shared_ptr<Log<HintMessage>> m_hint_log;

  std::unordered_map<std::string, pending_read> m_pending_reads;
  boost::asio::strand m_pending_read_strand{m_io_service};

  std::unordered_map<std::string, pending_write> m_pending_writes;
  boost::asio::strand m_pending_write_strand{m_io_service};

  std::vector<std::array<std::atomic_int_fast32_t, 256>> m_hints{3};

  boost::uuids::random_generator m_uuid_gen{};

  boost::asio::ip::tcp::acceptor m_acceptor{m_io_service};
  boost::asio::ip::tcp::socket m_socket{m_io_service};

  boost::asio::ip::tcp::acceptor m_client_acceptor{m_io_service};
  boost::asio::ip::tcp::socket m_client_socket{m_io_service};
};
}  // namespace kvstore::server
