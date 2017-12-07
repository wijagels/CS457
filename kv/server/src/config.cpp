#include "config.hpp"
#include <fstream>
#include <json/json.h>

namespace kvstore::server {
Config::Config(const stdfs::path &config_path) {
  std::ifstream file{config_path};
  Json::Value root;
  file >> root;
  m_listen_ip = root["listen_ip"].asString();
  m_listen_port = root["listen_port"].asUInt();
  m_client_port = root["client_port"].asUInt();
  m_log_path = root["logfile"].asString();
  m_hint_log = root["hintlog"].asString();
  for (const auto &replica : root["replicas"]) {
    m_replicas.emplace_back(replica["host"].asString(), replica["port"].asString());
  }
  m_mode = static_cast<Mode>(root["mode"].asInt());
}

const std::string &Config::listen_ip() const { return m_listen_ip; }

uint16_t Config::listen_port() const { return m_listen_port; }

uint16_t Config::client_port() const { return m_client_port; }

const stdfs::path &Config::log_path() const { return m_log_path; }

const stdfs::path &Config::hint_log() const { return m_hint_log; }

const std::vector<std::pair<std::string, std::string>> &Config::replicas() const {
  return m_replicas;
}

Config::Mode Config::mode() const { return m_mode; }
}  // namespace kvstore::server
