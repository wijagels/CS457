#pragma once
#include <utility>
#include <vector>
#if __has_include(<filesystem>)
#include <filesystem>
#else
#include <experimental/filesystem>
#endif

namespace kvstore::server {
#if __has_include(<filesystem>)
namespace stdfs = std::filesystem;
#else
namespace stdfs = std::experimental::filesystem;
#endif

class Config {
 public:
  enum class Mode { read_repair = 0, hinted_handoff = 1 };

  Config(const stdfs::path &config_path);
  const std::string &listen() const;
  const stdfs::path &log_path() const;
  const std::vector<std::pair<std::string, std::string>> &replicas() const;
  Mode mode() const;

 private:
  std::string m_listen;
  stdfs::path m_log_path;
  std::vector<std::pair<std::string, std::string>> m_replicas;
  Mode m_mode;
};
}  // namespace kvstore::server
