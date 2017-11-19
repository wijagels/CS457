#include "log.hpp"

namespace kvstore::server {
Log::Log(stdfs::path file_path, boost::asio::io_service &io_service)
    : m_file_path{std::move(file_path)},
      m_file{m_file_path, std::ios::out | std::ios::binary | std::ios::ate | std::ios::app},
      m_io_service{io_service},
      m_strand{m_io_service} {}

void Log::clear() {
  m_file.close();
  m_file.open(m_file_path, std::ios::out | std::ios::binary | std::ios::ate | std::ios::trunc);
}
}  // namespace kvstore::server
