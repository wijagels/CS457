#include "config.hpp"
#include <fstream>
#include <json/json.h>

namespace kvstore::server {
Config::Config(const stdfs::path &config_path) {
  std::ifstream file{config_path};
  Json::Value root;
  file >> root;
  m_listen = root["listen"].asString();
  m_log_path = root["logfile"].asString();
  for (const auto &replica : root["replicas"]) {
    m_replicas.emplace_back(replica["host"].asString(), replica["port"].asString());
  }
  m_mode = static_cast<Mode>(root["mode"].asInt());
}

const std::string &Config::listen() const { return m_listen; }

const stdfs::path &Config::log_path() const { return m_log_path; }

const std::vector<std::pair<std::string, std::string>> &Config::replicas() const {
  return m_replicas;
}

Config::Mode Config::mode() const { return m_mode; }
}  // namespace kvstore::server
