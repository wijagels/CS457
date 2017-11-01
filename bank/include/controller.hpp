#ifndef INCLUDE_CONTROLLER_HPP_
#define INCLUDE_CONTROLLER_HPP_
#include "channel.hpp"
#include "proto/bank.pb.h"
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <memory>
#include <string_view>
#include <tuple>
#include <vector>

class Controller : public std::enable_shared_from_this<Controller> {
 public:
  Controller(uint64_t initial_bal, boost::asio::io_service &io_service);

  void start(const std::string_view &filename);

 private:
  void do_read_list(const std::string_view &filename);

  void do_init_branches();

  void do_snapshot();

  void do_retrieve_snapshot();

  void handle_message(const BranchMessage &msg);

  void return_snapshot_handler(const ReturnSnapshot &msg);

  using peerinfo = std::tuple<std::string, std::string, uint16_t>;
  std::vector<peerinfo> d_peers;
  std::vector<std::shared_ptr<Channel>> d_channels;
  InitBranch d_msg;
  boost::asio::io_service &d_io_service;
  boost::asio::steady_timer d_timer;
  boost::asio::ip::tcp::resolver d_resolver;
  uint64_t d_snapshot_id = 0;
};

#endif
