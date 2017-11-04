#include "controller.hpp"
#include "proto/bank.pb.h"
#include <boost/asio.hpp>
#include <chrono>
#include <fstream>
#include <google/protobuf/text_format.h>
#include <memory>
#include <random>
#include <regex>
#include <string>
#if __has_include(<filesystem>)
#include <filesystem>
namespace stdfs = std::filesystem;
#else
#include <experimental/filesystem>
namespace stdfs = std::experimental::filesystem;
#endif

Controller::Controller(uint64_t initial_bal, boost::asio::io_service &io_service)
    : d_io_service{io_service}, d_timer{io_service}, d_resolver{io_service} {
  d_msg.set_balance(initial_bal);
}

void Controller::start(const std::string_view &filename) { do_read_list(filename); }

void Controller::do_read_list(const std::string_view &filename) {
  auto self = shared_from_this();
  d_io_service.post([this, self, filename]() {
    stdfs::path path{filename};
    if (!stdfs::exists(path) || !stdfs::is_regular_file(path)) {
      throw std::invalid_argument{"Invalid branches file"};
    }
    std::ifstream file_stream{path};
    if (!file_stream.is_open()) {
      throw std::invalid_argument{"Can't open branches file"};
    }
    std::regex line_re{R"((\S+)\s+(\S+)\s+(\d+))"};
    std::smatch match;
    for (std::string line; std::getline(file_stream, line);) {
      if (!std::regex_match(line, match, line_re) || match.size() != 4) {
        throw std::runtime_error{"Invalid format in input file"};
      }
      d_peers.emplace_back(match[1], match[2], std::stoul(match[3]));
      auto branch = d_msg.add_all_branches();
      branch->set_name(std::get<0>(d_peers.back()));
      branch->set_ip(std::get<1>(d_peers.back()));
      branch->set_port(std::get<2>(d_peers.back()));
    }
    do_init_branches();
  });
}

void Controller::do_init_branches() {
  auto self = shared_from_this();
  using boost::asio::ip::tcp;
  for (const auto & [ name, ip, port ] : d_peers) {
    auto ch = std::make_shared<Channel>(
        d_io_service, [this, self](const BranchMessage &msg) { this->handle_message(msg); });
    d_channels.emplace_back(ch);
    tcp::resolver::query query{tcp::v4(), ip, std::to_string(static_cast<uint32_t>(port)),
                               tcp::resolver::query::numeric_service};
    d_resolver.async_resolve(
        query, [this, self, ch](boost::system::error_code ec, tcp::resolver::iterator peers) {
          if (!ec) {
            ch->connect_cb(peers, [this, self, ch]() {
              BranchMessage msg;
              msg.mutable_init_branch()->CopyFrom(d_msg);
              ch->send_branch_msg(msg);
            });
          } else {
            throw std::runtime_error{"Unable to resolve peer: " + ec.message()};
          }
        });
  }
  do_snapshot();
}

void Controller::do_snapshot() {
  ++d_snapshot_id;
  auto self = shared_from_this();
  d_timer.expires_from_now(std::chrono::seconds(5));
  d_timer.async_wait([this, self](boost::system::error_code ec) {
    if (!ec) {
      BranchMessage bm;
      auto init_snap = bm.mutable_init_snapshot();
      init_snap->set_snapshot_id(d_snapshot_id);

      std::random_device rd;
      std::mt19937 gen{rd()};
      std::uniform_int_distribution<size_t> dist{0, d_channels.size() - 1};
      size_t which = dist(gen);
      d_channels.at(which)->send_branch_msg(bm);
      do_retrieve_snapshot();
    }
  });
}

void Controller::do_retrieve_snapshot() {
  auto self = shared_from_this();
  d_timer.expires_from_now(std::chrono::seconds(10));
  d_timer.async_wait([this, self](boost::system::error_code ec) {
    if (!ec) {
      std::cout << "snapshot_id: " << d_snapshot_id << '\n';
      BranchMessage bm;
      auto retrieve = bm.mutable_retrieve_snapshot();
      retrieve->set_snapshot_id(d_snapshot_id);
      for (auto &branch : d_channels) {
        branch->send_branch_msg(bm);
      }
      do_snapshot();
    }
  });
}

void Controller::handle_message(const BranchMessage &msg) {
  using MsgTypes = BranchMessage::BranchMessageCase;
  MsgTypes branch_msg = msg.branch_message_case();
  switch (branch_msg) {
    case MsgTypes::kInitBranch:
      return;
    case MsgTypes::kInitSnapshot:
      return;
    case MsgTypes::kMarker:
      return;
    case MsgTypes::kRetrieveSnapshot:
      return;
    case MsgTypes::kReturnSnapshot:
      return return_snapshot_handler(msg.return_snapshot());
    case MsgTypes::kTransfer:
      return;
    default:  // Also MsgTypes::BRANCH_MESSAGE_NOT_SET
      return;
  }
}

void Controller::return_snapshot_handler(const ReturnSnapshot &msg) {
  const auto &snap = msg.local_snapshot();
  const auto &name = msg.name();
  std::cout << name << ": " << snap.balance();
  for (int i = 0; i < snap.channel_state_size(); i++) {
    const auto &row = snap.channel_state(i);
    std::cout << ", " << row.name() << "->" << name << ": " << row.in_transit();
  }
  std::cout << '\n';
}
