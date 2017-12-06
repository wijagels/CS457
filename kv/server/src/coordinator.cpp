#include "coordinator.hpp"
#include <atomic>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <google/protobuf/util/time_util.h>
#include <memory>
#include <utility>

namespace kvstore::server {
Coordinator::Coordinator(boost::asio::io_service &io_service, Config cfg)
    : m_io_service{io_service},
      m_config{std::move(cfg)},
      m_replica{std::make_shared<Replica>(m_io_service, m_config)} {}

void Coordinator::start() {
  using boost::asio::ip::address;
  using boost::asio::ip::tcp;
  tcp::endpoint endpoint{address::from_string(m_config.listen_ip()),
                         static_cast<uint16_t>(m_config.listen_port())};
  std::cout << endpoint.address() << '\n';
  std::cout << endpoint.port() << '\n';
  m_acceptor.open(endpoint.protocol());
  m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  m_acceptor.bind(endpoint);
  m_acceptor.listen();
  initialize_outgoing();
  do_accept();
}

void Coordinator::do_accept() {
  auto self = shared_from_this();
  m_acceptor.async_accept(m_socket, [this, self](boost::system::error_code ec) {
    if (!ec) {
      auto ch = std::make_shared<Channel<ServerMessage>>(
          std::move(m_socket), m_io_service,
          [this, self](auto... x) { unintroduced_msg_handler(std::forward<decltype(x)>(x)...); });
      ch->start();
    }
    do_accept();
  });
}

void Coordinator::handle_read_repair_response(const ReadRepairResponse &msg,
                                              const std::shared_ptr<Channel<ServerMessage>> &from) {
  auto key = msg.key();
  auto my_copy = m_replica->read(key);
  if (my_copy->timestamp < msg.timestamp()) {
    m_replica->write(key, msg.val(), msg.timestamp());
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

void Coordinator::handle_read_repair(const ReadRepair &msg,
                                     const std::shared_ptr<Channel<ServerMessage>> &from) {
  auto data = m_replica->read(msg.key());
  ServerMessage reply;
  auto payload = reply.mutable_read_repair_response();
  payload->set_key(msg.key());
  payload->set_val(data->data);
  payload->mutable_timestamp()->CopyFrom(data->timestamp);
  from->send_msg(reply);
}

void Coordinator::handle_get(const client::GetKey &msg,
                             const std::shared_ptr<Channel<client::ClientMessage>> &from) {
  auto consistency = msg.consistency();
  auto data = m_replica->read(msg.key());
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
  if (consistency == 1) {
    client::ClientMessage reply;
    auto payload = reply.mutable_get_key_resp();
    payload->set_stream(msg.stream());
    payload->set_val(data->data);
    from->send_msg(reply);
  } else {
    m_pending_read_strand.dispatch([this, self, msg, from, data]() {
      auto uuid = boost::uuids::to_string(m_uuid_gen());
      m_pending_reads.emplace(
          std::piecewise_construct, std::forward_as_tuple(uuid),
          std::forward_as_tuple(msg.stream(), msg.key(), msg.consistency() - 1, from,
                                std::vector{std::pair{data->timestamp, data->data}}));
    });
  }
}

void Coordinator::handle_put(const client::PutKey &msg,
                             const std::shared_ptr<Channel<client::ClientMessage>> &from) {
  auto stamp = ::google::protobuf::util::TimeUtil::GetCurrentTime();
  m_replica->write(msg.key(), msg.val(), stamp);
  auto self = shared_from_this();

  if (m_config.mode() == Config::Mode::hinted_handoff) {
    auto key = msg.key();
    for (size_t i = 0; i < m_hints.size(); i++) {
      HintMessage hint_add;
      hint_add.mutable_add_hint()->set_key(key);
      hint_add.mutable_add_hint()->set_replica(i);
      m_hint_log.write_update(hint_add, [this, self, i, key]() { ++m_hints[i][key]; });
    }
  }

  if (msg.consistency() != 1) {
    m_pending_write_strand.dispatch([this, self, msg, from]() {
      auto uuid = boost::uuids::to_string(m_uuid_gen());
      m_pending_writes.emplace(
          std::piecewise_construct, std::forward_as_tuple(uuid),
          std::forward_as_tuple(msg.stream(), msg.key(), msg.consistency() - 1, from));
    });
  }
  ServerMessage server_msg;
  auto write_msg = server_msg.mutable_do_write();
  write_msg->set_key(msg.key());
  write_msg->set_val(msg.val());
  write_msg->set_uuid(boost::uuids::to_string(m_uuid_gen()));
  write_msg->mutable_timestamp()->CopyFrom(stamp);
  for (auto &ch : m_channels) {
    auto chan = std::atomic_load(&ch);
    if (chan) chan->send_msg(server_msg);
  }
}

void Coordinator::handle_write_complete(const server::WriteComplete &msg, uint32_t from) {
  m_pending_write_strand.dispatch([this, msg, from]() {
    auto &pending = m_pending_writes.at(msg.uuid());
    if (pending.consistency == 1) {
      client::ClientMessage reply;
      reply.mutable_put_key_resp()->set_stream(pending.stream);
      reply.mutable_put_key_resp()->set_key(pending.key);
      pending.requestor->send_msg(reply);
    }
    --pending.consistency;
    if (m_config.mode() == Config::Mode::hinted_handoff) {
      --m_hints[from][pending.key];
    }
  });
}

void Coordinator::handle_do_write(const DoWrite &msg,
                                  const std::shared_ptr<Channel<ServerMessage>> &ptr,
                                  uint32_t from) {
  m_replica->write(msg.key(), msg.val(), msg.timestamp());
  ServerMessage reply;
  reply.mutable_write_complete()->set_uuid(msg.uuid());
  m_channels[from]->send_msg(reply);
}

void Coordinator::handle_do_read(const DoRead &msg,
                                 const std::shared_ptr<Channel<ServerMessage>> &ptr,
                                 uint32_t /*from*/) {
  auto data = m_replica->read(msg.key());
  ServerMessage reply;
  auto *read_complete = reply.mutable_read_complete();
  read_complete->set_uuid(msg.uuid());
  read_complete->set_val(data->data);
  read_complete->mutable_timestamp()->CopyFrom(data->timestamp);
  ptr->send_msg(reply);
}

void Coordinator::handle_read_complete(const ReadComplete &msg,
                                       const std::shared_ptr<Channel<ServerMessage>> &ptr,
                                       uint32_t from) {
  m_pending_read_strand.dispatch([this, msg, from, ptr]() {
    auto &pending = m_pending_reads.at(msg.uuid());
    if (pending.consistency == 1) {
      client::ClientMessage reply;
      reply.mutable_get_key_resp()->set_stream(pending.stream);
      reply.mutable_get_key_resp()->set_key(pending.key);
      auto result = pending.results.front();
      for (const auto & [ stamp, val ] : pending.results) {
        if (result.first < stamp) {
          result.first = stamp;
          result.second = val;
        }
      }
      reply.mutable_get_key_resp()->set_val(result.second);
      pending.requestor->send_msg(reply);
    }
    --pending.consistency;
    if (m_config.mode() == Config::Mode::hinted_handoff) {
      --m_hints[from][pending.key];
    }
  });
}

void Coordinator::unintroduced_msg_handler(const ServerMessage &msg,
                                           const std::shared_ptr<Channel<ServerMessage>> &ptr) {
  if (!msg.has_intro()) throw std::runtime_error{"Replica failed to introduce self"};

  const auto &intro = msg.intro();
  uint32_t id = 0;
  bool found = false;
  for (uint32_t i = 0; i < m_config.replicas().size(); i++) {
    const auto & [ ip, port ] = m_config.replicas()[i];
    if (intro.ip() == ip && std::to_string(intro.port()) == port) {
      id = i;
      found = true;
      break;
    }
  }
  if (!found) throw std::runtime_error{"Invalid introduction"};
  ptr->handler() = [this, id](const ServerMessage &server_msg,
                              const std::shared_ptr<Channel<ServerMessage>> &server_ptr) {
    server_msg_handler(server_msg, server_ptr, id);
  };
}

void Coordinator::server_msg_handler(const ServerMessage &msg,
                                     const std::shared_ptr<Channel<ServerMessage>> &ptr,
                                     uint32_t id) {
  switch (msg.msg_case()) {
    case ServerMessage::kIntro:
      throw std::runtime_error{"Double introduction"};
    case ServerMessage::kDoWrite:
      return handle_do_write(msg.do_write(), ptr, id);
    case ServerMessage::kWriteComplete:
      return handle_write_complete(msg.write_complete(), id);
    case ServerMessage::kDoRead:
      return handle_do_read(msg.do_read(), ptr, id);
    case ServerMessage::kReadComplete:
      return handle_read_complete(msg.read_complete(), ptr, id);
    case ServerMessage::kReadRepair:
      return handle_read_repair(msg.read_repair(), ptr);
    case ServerMessage::kReadRepairResponse:
      return handle_read_repair_response(msg.read_repair_response(), ptr);
    default:
      std::cerr << "Unknown message type received\n";
  }
}

void Coordinator::initialize_outgoing() {
  using boost::asio::ip::tcp;
  tcp::resolver resolver{m_io_service};
  uint32_t id = 0;
  for (const auto & [ ip, port ] : m_config.replicas()) {
    auto handler = [this, id](const ServerMessage &msg,
                              const std::shared_ptr<Channel<ServerMessage>> &ptr) {
      server_msg_handler(msg, ptr, id);
    };
    auto new_chan =
        m_channels.emplace_back(std::make_shared<Channel<ServerMessage>>(m_io_service, handler));
    tcp::resolver::query query{ip, port, tcp::resolver::query::numeric_service};
    auto result = resolver.resolve(query);
    new_chan->connect(result);
    ++id;
  }
}
}  // namespace kvstore::server
