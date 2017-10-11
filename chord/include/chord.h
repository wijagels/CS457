#ifndef chord__H
#define chord__H
#include "FileStore.h"
#include "chord.h"
#include "chord_types.h"
#include "sha256.hpp"
#include <boost/make_shared.hpp>
#include <boost/optional.hpp>
#include <thrift/TToString.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <vector>

class LocalNode : public NodeID {
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
    auto successor = d_fingertable.front();
    call_rpc(successor.ip, successor.port, &FileStoreClient::setPredecessor, node);
  }

  template <typename Node, typename = std::enable_if_t<std::is_assignable<NodeID, Node>::value>>
  void set_predecessor(Node &&node) noexcept(std::is_nothrow_assignable<NodeID, Node>::value) {
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
    if (d_fingertable.empty()) {
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
      se.__set_message("No fingertable, can't find successor");
      throw se;
    }
    auto node = closest_preceding_node(_key);
    call_rpc(node.ip, node.port, &FileStoreClient::findSucc, _return, _key);
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
    auto node = closest_preceding_node(_key);
    call_rpc(node.ip, node.port, &FileStoreClient::findPred, _return, _key);
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

  static bool id_between(const std::string &lower, const std::string &middle,
                         const std::string &upper) {
    if (lower < upper) return lower < middle && middle < upper;
    return !(upper < middle && middle < lower);
  }

  NodeID closest_preceding_node(const std::string &_key) {
    for (auto iter = d_fingertable.rbegin(); iter != d_fingertable.rend(); ++iter) {
      auto node = *iter;
      if (id_between(this->id, node.id, _key)) {
        return node;
      }
    }
    return d_fingertable.front();
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
    transport->open();
    // Pointer to member function magic
    (client.*fn)(std::forward<Args>(args)...);
    transport->close();
  }
};

#endif
