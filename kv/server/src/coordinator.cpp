#include "coordinator.hpp"
#include <atomic>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <memory>
#include <utility>

namespace kvstore::server {
Coordinator::Coordinator(boost::asio::io_service &io_service, Config cfg)
    : m_io_service{io_service}, m_config{std::move(cfg)}, m_replica{m_io_service, m_config} {}

void Coordinator::do_read(const DoRead &msg) {
  auto self = shared_from_this();
  if (m_config.mode() == Config::Mode::read_repair) {
    m_io_service.post([this, self, msg]() {
      ServerMessage rrmsg;
      rrmsg.mutable_read_repair()->set_key(msg.key());
      for (auto &chp : m_channels) {
        auto ch = std::atomic_load(&chp);
        if (ch) {
          ch->send_msg(rrmsg);
        }
      }
    });
  }
}

static bool operator<(const ::google::protobuf::Timestamp &lhs,
                      const ::google::protobuf::Timestamp &rhs) {
  if (lhs.seconds() < rhs.seconds()) return true;
  if (lhs.seconds() == rhs.seconds()) return lhs.nanos() < rhs.nanos();
  return false;
}

void Coordinator::handle_read_repair_response(const ReadRepairResponse &msg,
                                              std::shared_ptr<Channel<ServerMessage>> from) {
  auto key = msg.key();
  auto my_copy = m_replica.read(key);
  if (my_copy->timestamp < msg.timestamp()) {
    m_replica.write(key, msg.val(), msg.timestamp());
    ServerMessage server_msg;
    auto write_msg = server_msg.mutable_do_write();
    write_msg->set_uuid("");
    write_msg->set_key(key);
    write_msg->set_val(msg.val());
    write_msg->mutable_timestamp()->CopyFrom(msg.timestamp());
    for (auto &ch : m_channels) {
      auto chan = std::atomic_load(&ch);
      if (chan) chan->send_msg(server_msg);
    }
  } else if (msg.timestamp() < my_copy->timestamp || my_copy->data != msg.val()) {
    ServerMessage server_msg;
    auto write_msg = server_msg.mutable_do_write();
    write_msg->set_uuid("");
    write_msg->set_key(msg.key());
    write_msg->set_val(my_copy->data);
    write_msg->mutable_timestamp()->CopyFrom(my_copy->timestamp);
    from->send_msg(server_msg);
  }
}

void Coordinator::handle_get(const client::GetKey &msg,
                             std::shared_ptr<Channel<client::ClientMessage>> from) {
  auto consistency = msg.consistency();
  auto data = m_replica.read(msg.key());
  if (consistency == 1) {
    client::ClientMessage reply;
    auto payload = reply.mutable_get_key_resp();
    payload->set_stream(msg.stream());
    payload->set_val(data->data);
    from->send_msg(reply);
  } else {
    auto self = shared_from_this();
    m_pending_read_strand.dispatch([this, self, msg, from, data]() {
      auto uuid = boost::uuids::to_string(m_uuid_gen());
      m_pending_reads.emplace(
          std::piecewise_construct, std::forward_as_tuple(uuid),
          std::forward_as_tuple(msg.stream(), msg.key(), msg.consistency(), from,
                                std::vector{std::pair{data->timestamp, data->data}}));
      // lol
    });
  }
}
}  // namespace kvstore::server
