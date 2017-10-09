#include "FileStore.h"
#include "chord.h"
#include "chord_types.h"
#include <boost/make_shared.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <thrift/TToString.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/TFDTransport.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

class FileStoreHandler : virtual public FileStoreIf {
  LocalNode d_node;

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
  auto handler = make_shared<FileStoreHandler>("127.0.0.1", port);
  auto processor = make_shared<FileStoreProcessor>(handler);
  auto serverTransport = make_shared<TServerSocket>(port);
  auto transportFactory = make_shared<TBufferedTransportFactory>();
  auto protocolFactory = make_shared<TBinaryProtocolFactory>();

  TThreadedServer server{processor, serverTransport, transportFactory, protocolFactory};
  server.serve();
}
