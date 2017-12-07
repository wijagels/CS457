#include "replica.hpp"
#include <google/protobuf/util/time_util.h>

namespace kvstore::server {
Replica::Replica(boost::asio::io_service &io_svc, Config cfg)
    : m_io_service{io_svc},
      m_store{},
      m_config{std::move(cfg)},
      m_log{std::make_shared<Log<DoWrite>>(m_config.log_path(), m_io_service)} {
  m_log->replay_log([this](const DoWrite &msg) { replay_handler(msg); });
}

std::shared_ptr<Replica::storage_element> Replica::read(uint32_t key) {
  auto ptr = std::atomic_load(&m_store[key]);
  return ptr;
}

void Replica::write(uint32_t key, const std::string &data,
                    const google::protobuf::Timestamp &timestamp) {
  auto self = shared_from_this();
  auto current = std::atomic_load(&m_store[key]);
  if (current && timestamp <= current->timestamp) return;
  auto new_element = std::make_shared<storage_element>(data, timestamp);
  DoWrite msg;
  msg.set_key(key);
  msg.set_val(data);
  msg.mutable_timestamp()->CopyFrom(timestamp);
  auto *slot = &m_store[key];
  m_log->write_update(msg, [self, timestamp, new_element, slot]() {
    auto dest = std::atomic_load(slot);
    for (;;) {
      if (dest && timestamp <= dest->timestamp) return;
      if (std::atomic_compare_exchange_weak(slot, &dest, new_element)) return;
    }
  });
}

void Replica::replay_handler(const DoWrite &msg) {
  write(msg.key(), msg.val(), msg.timestamp());
}
}  // namespace kvstore::server
