#ifndef chord__H
#define chord__H
#include "FileStore.h"
#include "chord.h"
#include "chord_types.h"
#include "sha256.hpp"
#include <boost/make_shared.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/optional.hpp>
#include <thrift/TToString.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <vector>

class LocalNode : NodeID {
 public:
  LocalNode(const std::string &address, int m_port) {
    NodeID::ip = address;
    NodeID::port = m_port;
    NodeID::id = sha256(ip + ":" + std::to_string(port));
  }

  template <typename Vec,
            typename = std::enable_if_t<std::is_assignable<std::vector<NodeID>, Vec>::value>>
  void set_fingertable(Vec &&node_list) noexcept(
      std::is_nothrow_assignable<std::vector<NodeID>, Vec>::value) {
    d_fingertable = std::forward<Vec>(node_list);
    for (auto &n : d_fingertable) {
      n.id = sha256(n.ip + ":" + std::to_string(n.port));
    }
    NodeID node = *this;
    call_rpc(d_fingertable.front().ip, d_fingertable.front().port, &FileStoreClient::setPredecessor,
             node);
  }

  template <typename Node, typename = std::enable_if_t<std::is_assignable<NodeID, Node>::value>>
  void set_predecessor(Node &&node) noexcept(std::is_nothrow_assignable<NodeID, Node>::value) {
    std::cout << node << std::endl;
    d_predecessor = std::forward<Node>(node);
  }

  bool is_successor(const std::string &identifier) {
    if (!d_predecessor) {
      SystemException se{};
      se.__set_message("Unknown predecessor, can't determine if is successor");
      throw se;
    }
    return id_between(d_predecessor->id, identifier, this->id);
  }

  bool is_predecessor(const std::string &identifier) {
    if (!d_fingertable.empty()) {
      SystemException se{};
      se.__set_message("Unknown successor, can't determine if is predecessor");
      throw se;
    }
    return id_between(this->id, identifier, d_fingertable.front().id);
  }

  void find_successor(NodeID &_return, const std::string &_key) {
    if (is_successor(_key)) {
      _return = *this;
      return;
    }
    if (d_fingertable.empty()) {
      SystemException se{};
      se.__set_message("Unknown successor, can't find successor");
      throw se;
    }
    auto &candidate = d_fingertable.front();
    for (const auto &node : d_fingertable) {
      if (id_less(candidate.id, _key)) break;
      candidate = node;
    }
    call_rpc(candidate.ip, candidate.port, &FileStoreClient::findSucc, _return, _key);
  }

  void find_predecessor(NodeID &_return, const std::string &_key) {
    if (is_predecessor(_key)) {
      _return = *this;
      return;
    }
    if (d_fingertable.empty()) {
      SystemException se{};
      se.__set_message("Unknown successor, can't find successor");
      throw se;
    }
    auto &candidate = d_fingertable.front();
    for (const auto &node : d_fingertable) {
      candidate = node;
      if (id_less(candidate.id, _key)) break;
    }
    call_rpc(candidate.ip, candidate.port, &FileStoreClient::findPred, _return, _key);
  }

  void get_successor(NodeID &_return) {
    if (d_fingertable.empty()) {
      SystemException se{};
      se.__set_message("Empty fingertable, can't get successor");
      throw se;
    }
    _return = d_fingertable.front();
  }

 private:
  std::vector<NodeID> d_fingertable;
  boost::optional<NodeID> d_predecessor;

  static bool id_less(const std::string &lhs, const std::string &rhs) {
    using boost::multiprecision::uint256_t;
    uint256_t lhi{"0x" + lhs};
    uint256_t rhi{"0x" + rhs};
    return lhi < rhi;
  }

  static bool id_between(const std::string &f, const std::string &s, const std::string &t) {
    using boost::multiprecision::uint256_t;
    uint256_t fi{"0x" + f};
    uint256_t si{"0x" + s};
    uint256_t ti{"0x" + t};
    if (fi < si) return fi < ti && ti < si;
    return !(si < ti && ti < fi);
  }

  template <class Fp, typename... Args>
  void call_rpc(const std::string &dest_ip, int dest_port, Fp fn, Args &&... args) {
    using namespace ::apache::thrift;
    using namespace ::apache::thrift::protocol;
    using namespace ::apache::thrift::transport;
    auto socket = boost::make_shared<TSocket>(dest_ip, dest_port);
    auto transport = boost::make_shared<TBufferedTransport>(socket);
    auto protocol = boost::make_shared<TBinaryProtocol>(transport);
    FileStoreClient client(protocol);
    auto f = std::bind(fn, &client, (std::ref(args))...);
    transport->open();
    f();
    transport->close();
  }
};

#endif