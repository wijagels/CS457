#include "replica.hpp"

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
}  // namespace kvstore::server
