#include "replica.hpp"
#include <google/protobuf/util/time_util.h>

namespace kvstore::server {
Replica::Replica(boost::asio::io_service &io_svc, Config cfg)
    : m_io_service{io_svc},
      m_store{},
      m_config{std::move(cfg)},
      m_log{m_config.log_path(), m_io_service} {}

std::shared_ptr<Replica::storage_element> Replica::read(uint32_t key) {
  auto ptr = std::atomic_load(&m_store[key]);
  return ptr;
}

void Replica::write(uint32_t key, const std::string &data,
                    const google::protobuf::Timestamp &timestamp) {
  auto current = std::atomic_load(&m_store[key]);
  if (timestamp <= current->timestamp) return;
  auto new_element = std::make_shared<storage_element>(data, timestamp);
  for (;;) {
    if (timestamp <= current->timestamp) return;
    if (std::atomic_compare_exchange_weak(&m_store[key], &current, new_element)) return;
  }
}
}  // namespace kvstore::server
