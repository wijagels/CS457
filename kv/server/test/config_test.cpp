#include "config.hpp"
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <utility>

using kvstore::server::Config;

TEST(Config, load_test) {
  using namespace std::literals::string_literals;
  std::ofstream file{"test_cfg.json"};
  file << R"({"listen_ip": "127.0.0.1",
  "listen_port": 8080,
  "logfile": "test.log",
  "replicas": [ {"host": "localhost", "port": 9000},
  {"host": "example.com", "port": "9001"} ]})";
  file.close();
  Config cfg{"test_cfg.json"};
  EXPECT_EQ(cfg.listen_ip(), "127.0.0.1");
  EXPECT_EQ(cfg.listen_port(), 8080);
  EXPECT_EQ(cfg.log_path(), "test.log");
  EXPECT_NE(
      std::find(cfg.replicas().begin(), cfg.replicas().end(), std::pair{"example.com"s, "9001"s}),
      cfg.replicas().end());
  EXPECT_NE(
      std::find(cfg.replicas().begin(), cfg.replicas().end(), std::pair{"localhost"s, "9000"s}),
      cfg.replicas().end());
}
