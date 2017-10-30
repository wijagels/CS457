#include "branch.hpp"
#include "channel.hpp"
#include "netutils.hpp"
#include "proto/bank.pb.h"
#include <boost/asio.hpp>
#include <google/protobuf/text_format.h>
#include <iostream>
#include <unordered_map>

using boost::asio::ip::tcp;

Branch::Branch(std::string name, const tcp::endpoint &endpoint, boost::asio::io_service &io_service)
    : d_name{std::move(name)},
      d_acceptor{io_service, endpoint},
      d_socket{io_service},
      d_io_svc{io_service} {}

void Branch::start() { do_accept(); }

void Branch::do_accept() {
  std::cout << "Here!" << std::endl;
  auto self = shared_from_this();
  d_acceptor.async_accept(d_socket, [this, self](boost::system::error_code ec) {
    if (!ec) {
      auto ch = std::make_shared<Channel>(std::move(d_socket), self);
      d_pending_peers.push_back(ch);
      ch->start();
      match_peers();
    }
    do_accept();
  });
}

void Branch::match_peers() {
  auto it = std::remove_if(
      std::begin(d_pending_peers), std::end(d_pending_peers),
      [this](std::shared_ptr<Channel> &chan) {
        for (const auto & [ name, address, port ] : d_known_peers) {
          if (chan->peer().address() == boost::asio::ip::address::from_string(address) &&
              chan->peer().port() == port) {
            d_channels.emplace(name, chan);
            return true;
          }
        }
        return false;
      });
  d_pending_peers.erase(it, std::end(d_pending_peers));
}

void Branch::init_branch_handler(const InitBranch &msg) {
  auto self = shared_from_this();
  d_money = msg.balance();

  const size_t num = msg.all_branches_size();
  const size_t outgoing = (num - 1) / 2;

  d_known_peers.reserve(num);

  for (size_t i = 0; i < num; i++) {
    const auto &el = msg.all_branches(i);
    d_known_peers.emplace_back(el.name(), el.ip(), el.port());
  }
  std::sort(d_known_peers.begin(), d_known_peers.end(),
            [](const peer_info &lhs, const peer_info &rhs) {
              return std::get<0>(lhs) < std::get<0>(rhs);
            });

  auto iter = std::find_if(d_known_peers.begin(), d_known_peers.end(),
                           [=](const peer_info &x) { return std::get<0>(x) == d_name; });
  if (iter == d_known_peers.end())
    throw std::runtime_error{"Init branch message does not include me!"};
  const size_t offset = (iter - d_known_peers.begin());
  tcp::resolver resolver{d_io_svc};
  for (size_t i = offset; i < offset + outgoing; i++) {
    const auto & [ name, ip, port ] = d_known_peers.at(i % d_known_peers.size());
    auto ch = std::make_shared<Channel>(tcp::socket{d_io_svc}, self);
    d_channels.emplace(name, ch);
    tcp::endpoint peer{boost::asio::ip::address::from_string(ip), static_cast<uint16_t>(port)};
    tcp::resolver::query query{tcp::v4(), ip, std::to_string(static_cast<uint32_t>(port)),
                               tcp::resolver::query::numeric_service};
    resolver.async_resolve(query,
                           [ch](boost::system::error_code ec, tcp::resolver::iterator peers) {
                             if (!ec) {
                               ch->connect(peers);
                             } else {
                               throw std::runtime_error{"Unable to resolve peer: " + ec.message()};
                             }
                           });
  }
}

void Branch::init_snapshot_handler(const InitSnapshot &msg) {
  auto n = d_channels.size();
  auto id = msg.snapshot_id();
  auto bal = d_money.load(std::memory_order_relaxed);
  d_snapshot = Snapshot{n, id, bal};
}

void Branch::marker_handler(const Marker &msg) {
  std::string s;
  google::protobuf::TextFormat::PrintToString(msg, &s);
  std::cout << s << '\n';
}

void Branch::retrieve_snapshot_handler(const RetrieveSnapshot &msg) {
  std::string s;
  google::protobuf::TextFormat::PrintToString(msg, &s);
  std::cout << s << '\n';
}

void Branch::return_snapshot_handler(const ReturnSnapshot &msg) {
  std::string s;
  google::protobuf::TextFormat::PrintToString(msg, &s);
  std::cout << s << '\n';
}

void Branch::transfer_handler(const Transfer &msg) { d_money += msg.money(); }

void Branch::handle_message(const BranchMessage &msg) {
  using MsgTypes = BranchMessage::BranchMessageCase;
  MsgTypes branch_msg = msg.branch_message_case();
  switch (branch_msg) {
    case MsgTypes::kInitBranch:
      return init_branch_handler(msg.init_branch());
    case MsgTypes::kInitSnapshot:
      return init_snapshot_handler(msg.init_snapshot());
    case MsgTypes::kMarker:
      return marker_handler(msg.marker());
    case MsgTypes::kRetrieveSnapshot:
      return retrieve_snapshot_handler(msg.retrieve_snapshot());
    case MsgTypes::kReturnSnapshot:
      return return_snapshot_handler(msg.return_snapshot());
    case MsgTypes::kTransfer:
      return transfer_handler(msg.transfer());
    default:  // Also MsgTypes::BRANCH_MESSAGE_NOT_SET
      return;
  }
}
