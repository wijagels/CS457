#include "log.hpp"
#include <gtest/gtest.h>

using kvstore::server::Log;

TEST(Log, single_write) {  // NOLINT
  boost::asio::io_service svc;
  auto log = std::make_shared<Log>("testing.log", svc);
  log->clear();
  kvstore::server::ServerMessage msg;
  msg.mutable_do_write()->set_uuid("Hello world");
  auto runs = 0;
  log->write_update(msg, [&runs]() { runs++; });
  log->replay_log([&runs](kvstore::server::ServerMessage &&sm) {
    EXPECT_EQ(sm.do_write().uuid(), "Hello world") << "Deserialized data should match input";
    runs++;
  });
  svc.run();
  EXPECT_EQ(runs, 2) << "Both callbacks should happen";
}

TEST(Log, multi_write) {  // NOLINT
  boost::asio::io_service svc;
  auto log = std::make_shared<Log>("testing.log", svc);
  log->clear();
  auto runs = 0;
  kvstore::server::ServerMessage msg2;
  msg2.mutable_do_write()->set_uuid("Hello mundo");
  log->write_update(msg2, []() {});
  log->write_update(msg2, []() {});
  log->write_update(msg2, []() {});
  log->replay_log([&runs](kvstore::server::ServerMessage &&sm) {
    EXPECT_EQ(sm.do_write().uuid(), "Hello mundo");
    runs++;
  });
  svc.run();
  EXPECT_EQ(runs, 3) << "Replay should get 3 calls";
}
