#include "FileStore.h"
#include "chord.h"
#include "chord_types.h"
#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>
#include <mutex>
#include <thrift/TToString.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TFDTransport.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include <cstdio>
extern "C" {
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
}

class FileStoreHandler : virtual public FileStoreIf {
  LocalNode d_node;
  std::recursive_mutex d_file_mtx;  // A thoroughly lazy approach to fixing race conditions

 public:
  FileStoreHandler(const std::string& address, int port) : d_node{address, port} {}

  void writeFile(const RFile& rFile) {
    printf("writeFile\n");
    RFile file{rFile};
    auto id = sha256(file.meta.filename + ":" + file.meta.owner);
    if (!d_node.is_successor(id)) {
      SystemException se{};
      se.__set_message("Node is not owner of file");
      throw se;
    }
    file.__isset.meta = true;
    file.meta.__set_contentHash(sha256(file.content));
    file.meta.__set_version(0);

    std::lock_guard<std::recursive_mutex> lock{d_file_mtx};  // Take lock a-la RAII

    if (boost::filesystem::exists(id)) {  // Read first to get the file version
      RFile old;
      readFile(old, file.meta.filename, file.meta.owner);
      file.meta.__set_version(old.meta.version + 1);
      boost::filesystem::remove(id);  // Delete file before writing
    }

    using namespace ::apache::thrift::transport;
    using namespace ::apache::thrift::protocol;
    auto transport = boost::make_shared<TFileTransport>(id);
    auto protocol = boost::make_shared<TBinaryProtocol>(transport);
    file.write(protocol.get());
  }

  void readFile(RFile& _return, const std::string& filename, const UserID& owner) {
    printf("readFile\n");
    auto id = sha256(filename + ":" + owner);
    if (!d_node.is_successor(id)) {
      SystemException se{};
      se.__set_message("Node is not owner of file");
      throw se;
    }

    std::lock_guard<std::recursive_mutex> lock{d_file_mtx};
    if (!boost::filesystem::exists(id)) {
      SystemException se{};
      se.__set_message("File does not exist");
      throw se;
    }
    using namespace ::apache::thrift::transport;
    using namespace ::apache::thrift::protocol;
    auto transport = boost::make_shared<TFileTransport>(id);
    auto protocol = boost::make_shared<TBinaryProtocol>(transport);
    _return.read(protocol.get());
  }

  void setFingertable(const std::vector<NodeID>& node_list) {
    printf("setFingertable\n");
    d_node.set_fingertable(node_list);
  }

  void findSucc(NodeID& _return, const std::string& key) {
    printf("findSucc\n");
    d_node.find_successor(_return, key);
  }

  void findPred(NodeID& _return, const std::string& key) {
    printf("findPred\n");
    d_node.find_predecessor(_return, key);
  }

  void getNodeSucc(NodeID& _return) {
    printf("getNodeSucc\n");
    d_node.get_successor(_return);
  }

  void setPredecessor(const NodeID& node) {
    printf("setPredecessor\n");
    d_node.set_predecessor(node);
  }
};

/**
 * Adapted from: https://stackoverflow.com/a/265978/1666415
 * This works well enough but it's kind of a hack.
 * A better way to do this would be with a configuration file,
 * especially since the node's identifier depends on its address.
 */
static std::string get_public_ip() {
  ifaddrs* ifAddrStruct = nullptr;
  ifaddrs* ifa = nullptr;
  in_addr* tmpAddrPtr = nullptr;

  getifaddrs(&ifAddrStruct);
  for (ifa = ifAddrStruct; ifa; ifa = ifa->ifa_next) {
    // We don't want interfaces with no address or loopback
    if (!ifa->ifa_addr || !std::strcmp(ifa->ifa_name, "lo")) continue;
    if (ifa->ifa_addr->sa_family == AF_INET) {
      tmpAddrPtr = &(reinterpret_cast<sockaddr_in*>(ifa->ifa_addr))->sin_addr;
      char addressBuffer[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
      return {addressBuffer};
    }
  }
  SystemException se;
  se.__set_message("Unable to find an address to bind to");
  throw se;
}

int main(int argc, char** argv) {
  using namespace ::apache::thrift;
  using namespace ::apache::thrift::protocol;
  using namespace ::apache::thrift::transport;
  using namespace ::apache::thrift::server;
  using boost::make_shared;

  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    exit(1);
  }
  int port = atoi(argv[1]);
  auto ip = get_public_ip();
  auto id = sha256(ip + ":" + std::to_string(port));
  printf("Server starting on %s:%d I am %s\n", ip.c_str(), port, id.c_str());
  auto handler = make_shared<FileStoreHandler>(ip.c_str(), port);
  auto processor = make_shared<FileStoreProcessor>(handler);
  auto serverTransport = make_shared<TServerSocket>(ip.c_str(), port);
  auto transportFactory = make_shared<TBufferedTransportFactory>();
  auto protocolFactory = make_shared<TBinaryProtocolFactory>();

  TThreadedServer server{processor, serverTransport, transportFactory, protocolFactory};
  server.serve();
}
