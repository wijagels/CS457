#define GTEST_HAS_TR1_TUPLE 1
#define GTEST_LANG_CXX11 0
#include "chord.h"
#include "FileStore.h"
#include "gtest/gtest.h"
#include <boost/make_shared.hpp>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <vector>

using ::apache::thrift::protocol::TBinaryProtocol;
using ::apache::thrift::transport::TBufferedTransport;
using ::apache::thrift::transport::TSocket;
using ::boost::make_shared;

/**
 * Simple check to see if any exceptions are thrown.
 * Need to manually check if it was actually the right node.
 */
TEST(chord_test, find_succ) {
  auto socket = make_shared<TSocket>("127.0.0.1", 9090);
  auto transport = make_shared<TBufferedTransport>(socket);
  auto protocol = make_shared<TBinaryProtocol>(transport);
  FileStoreClient client{protocol};
  transport->open();
  NodeID ret;
  client.findSucc(ret, "55cd64297b4aa7bda529d6c069cb479a0ceb6743b9540682649c16978c7b078c");
  transport->close();
  std::cout << "127.0.0.1" << ":" << ret.port << "\n";
}

/**
 * Check that we can write contents to a file and read them back correctly.
 */
TEST(chord_test, write_file) {
  auto socket = make_shared<TSocket>("127.0.0.1", 9091);
  auto transport = make_shared<TBufferedTransport>(socket);
  auto protocol = make_shared<TBinaryProtocol>(transport);
  FileStoreClient client{protocol};
  transport->open();
  {
    RFile file;
    file.__isset.meta = true;
    file.__set_content("Hello, world!");
    file.meta.__set_owner("guesttttttttt");
    file.meta.__set_filename("test.txt");
    client.writeFile(file);
  }
  {
    RFile file;
    client.readFile(file, "test.txt", "guest");
    EXPECT_EQ(file.content, "Hello, world!");
  }
  transport->close();
}
