#include "branch.hpp"
#include "channel.hpp"
#include "netutils.hpp"
#include "proto/bank.pb.h"
#include <boost/asio.hpp>
#include <chrono>
#include <google/protobuf/text_format.h>
#include <iostream>
#include <unordered_map>

using boost::asio::ip::tcp;

Branch::Branch(std::string name, const tcp::endpoint &endpoint, boost::asio::io_service &io_service)
    : c_name{std::move(name)},
      d_acceptor{io_service, endpoint},
      d_socket{io_service},
      d_io_service{io_service},
      d_timer{io_service},
      d_rd{},
      d_gen{d_rd()},
      d_resolver{io_service},
      d_strand{io_service} {}

void Branch::start() { do_accept(); }

void Branch::do_send_money() {
  auto self = shared_from_this();
  std::uniform_int_distribution<int> timer_dist{0, 5};
  d_timer.expires_from_now(std::chrono::milliseconds(timer_dist(d_gen)));
  d_timer.async_wait(d_strand.wrap([this, self](boost::system::error_code ec) {
    if (!ec && d_channels.size()) {
      std::uniform_int_distribution<size_t> chan_dist{0, d_channels.size() - 1};
      size_t i = chan_dist(d_gen);
      for (auto & [ name, el ] : d_channels) {
        if (i == 0) {
          BranchMessage bm;
          auto tr = bm.mutable_transfer();
          uint64_t curr_money = d_money.load(std::memory_order_relaxed);
          uint64_t to_send = 0;
          for (;;) {
            std::uniform_int_distribution<uint64_t> subtr_dist{
                static_cast<uint64_t>(0.01 * curr_money), static_cast<uint64_t>(0.05 * curr_money)};
            to_send = subtr_dist(d_gen);
            uint64_t end_bal = curr_money - to_send;
            if (to_send == 0) break;
            if (end_bal > curr_money) continue;
            if (d_money.compare_exchange_weak(curr_money, end_bal)) break;
            /* L O C K  F R E E */
          }
          if (to_send > 0) {
            tr->set_money(to_send);
            el->send_branch_msg(bm);
          }
          break;
        }
        --i;
      }
    }
    if (ec) std::cerr << ec.message() << std::endl;
    do_send_money();
  }));
}

void Branch::do_accept() {
  auto self = shared_from_this();
  d_acceptor.async_accept(d_socket, d_strand.wrap([this, self](boost::system::error_code ec) {
    if (!ec) {
      auto ch = std::make_shared<Channel>(
          std::move(d_socket), d_io_service,
          [this, self](const BranchMessage &msg) { message_handler(msg, {}); });
      std::pair p{"", ch};
      ch->handler() = [this, self, p](const BranchMessage &m) { message_handler(m, p); };
      d_pending_peers.push_back(ch);
      ch->start();
    }
    do_accept();
  }));
}

void Branch::init_branch_handler(const InitBranch &msg, const channel_info &) {
  auto self = shared_from_this();
  d_money = msg.balance();

  const size_t num = msg.all_branches_size();
  d_known_peers.reserve(num);

  for (size_t i = 0; i < num; i++) {
    const auto &el = msg.all_branches(i);
    d_known_peers.emplace_back(el.name(), el.ip(), el.port());
  }
  auto iter = std::find_if(d_known_peers.begin(), d_known_peers.end(),
                           [=](const peer_info &x) { return std::get<0>(x) == c_name; });
  if (iter == d_known_peers.end()) {
    throw std::runtime_error{"Init branch message does not include me!"};
  }

  const size_t offset = (iter - d_known_peers.begin());
  const size_t outgoing = static_cast<size_t>((num - 1) / 2) +
                          static_cast<size_t>((num % 2 == 0) ? (offset < (num / 2)) : 0);
  for (size_t i = offset + 1; i < offset + outgoing + 1; i++) {
    const auto & [ name, ip, port ] = d_known_peers.at(i % d_known_peers.size());
    auto ch = std::make_shared<Channel>(
        d_io_service, [this, self](const BranchMessage &m) { message_handler(m, {}); });
    std::pair p{name, ch};
    ch->handler() = [this, self, p](const BranchMessage &m) { message_handler(m, p); };
    tcp::resolver::query query{tcp::v4(), ip, std::to_string(static_cast<uint32_t>(port)),
                               tcp::resolver::query::numeric_service};
    d_resolver.async_resolve(query, [ this, name = name, ch, self ](boost::system::error_code ec,
                                                                    tcp::resolver::iterator peers) {
      if (!ec) {
        ch->connect_cb(peers, d_strand.wrap([this, name, ch, self]() {
          d_channels.emplace(name, ch);
          BranchMessage bm;
          bm.mutable_hello()->set_name(c_name);
          ch->send_branch_msg(bm);
        }));
      } else {
        throw std::runtime_error{"Unable to resolve peer: " + ec.message()};
      }
    });
  }
  d_known_peers.erase(iter);
  do_send_money();
}

void Branch::init_snapshot_handler(const InitSnapshot &msg, const channel_info &) {
  auto self = shared_from_this();
  d_strand.post([this, msg, self]() {
    auto n = d_channels.size();
    auto id = msg.snapshot_id();
    auto bal = d_money.load(std::memory_order_relaxed);
    d_snapshot = std::make_shared<Snapshot>(n, id, bal, d_known_peers);

    BranchMessage bm;
    bm.mutable_marker()->set_snapshot_id(id);
    for (auto & [ name, ch ] : d_channels) {
      ch->send_branch_msg(bm);
    }
  });
}

void Branch::marker_handler(const Marker &msg, const channel_info &ci) {
  auto self = shared_from_this();
  d_strand.post([this, msg, ci, self]() {
    std::string s;
    google::protobuf::TextFormat::PrintToString(msg, &s);
    if (!d_snapshot || msg.snapshot_id() != d_snapshot->id()) {
      d_snapshot =
          std::make_shared<Snapshot>(d_channels.size(), msg.snapshot_id(), d_money, d_known_peers);
      BranchMessage bm;
      bm.mutable_marker()->set_snapshot_id(msg.snapshot_id());
      for (auto & [ name, ch ] : d_channels) {
        ch->send_branch_msg(bm);
      }
    }
    d_snapshot->marker(ci.first);
  });
}

void Branch::retrieve_snapshot_handler(const RetrieveSnapshot &msg, const channel_info &ci) {
  auto self = shared_from_this();
  d_strand.post([this, msg, ci, self]() {
    if (msg.snapshot_id() == d_snapshot->id()) {
      BranchMessage bm;
      bm.mutable_return_snapshot()->CopyFrom(d_snapshot->to_message());
      bm.mutable_return_snapshot()->set_name(c_name);
      ci.second->send_branch_msg(bm);
    } else {
      std::cerr << "Someone requested a nonexistent snapshot" << std::endl;
    }
  });
}

void Branch::return_snapshot_handler(const ReturnSnapshot &, const channel_info &) {}

void Branch::transfer_handler(const Transfer &msg, const channel_info &ci) {
  d_money += msg.money();
  if (d_snapshot) {
    d_snapshot->record_tx(ci.first, msg.money());
  }
}

void Branch::hello_handler(const Hello &msg, const channel_info &ci) {
  auto self = shared_from_this();
  d_strand.post([this, msg, ci, self]() {
    auto it = std::remove(std::begin(d_pending_peers), std::end(d_pending_peers), ci.second);
    d_pending_peers.erase(it, std::end(d_pending_peers));
    std::pair p{msg.name(), ci.second};
    ci.second->handler() = [this, self, p](const BranchMessage &m) { message_handler(m, p); };
    d_channels.emplace(msg.name(), ci.second);
  });
}

void Branch::message_handler(const BranchMessage &msg, const channel_info &ci) {
  using MsgTypes = BranchMessage::BranchMessageCase;
  MsgTypes branch_msg = msg.branch_message_case();
  switch (branch_msg) {
    case MsgTypes::kInitBranch:
      return init_branch_handler(msg.init_branch(), ci);
    case MsgTypes::kInitSnapshot:
      return init_snapshot_handler(msg.init_snapshot(), ci);
    case MsgTypes::kMarker:
      return marker_handler(msg.marker(), ci);
    case MsgTypes::kRetrieveSnapshot:
      return retrieve_snapshot_handler(msg.retrieve_snapshot(), ci);
    case MsgTypes::kReturnSnapshot:
      return return_snapshot_handler(msg.return_snapshot(), ci);
    case MsgTypes::kTransfer:
      return transfer_handler(msg.transfer(), ci);
    case MsgTypes::kHello:
      return hello_handler(msg.hello(), ci);
    default:  // Also MsgTypes::BRANCH_MESSAGE_NOT_SET
      return;
  }
}
