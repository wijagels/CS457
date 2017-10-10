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
  for (int i = 9090; i <= 9099; i++) {
    auto socket = make_shared<TSocket>("10.202.6.29", i);
    auto transport = make_shared<TBufferedTransport>(socket);
    auto protocol = make_shared<TBinaryProtocol>(transport);
    FileStoreClient client{protocol};
    transport->open();
    NodeID ret;
    client.findSucc(ret, "3cf3e4d6eaf91f7db4fe6a0b3f3867652fe918c6e2f2978c12c3da36a79ebcff");
    transport->close();
    EXPECT_EQ(ret.port, 9092);
  }
}

TEST(chord_test, find_pred) {
  for (int i = 9090; i <= 9099; i++) {
    auto socket = make_shared<TSocket>("10.202.6.29", i);
    auto transport = make_shared<TBufferedTransport>(socket);
    auto protocol = make_shared<TBinaryProtocol>(transport);
    FileStoreClient client{protocol};
    transport->open();
    NodeID ret;
    client.findPred(ret, "3cf3e4d6eaf91f7db4fe6a0b3f3867652fe918c6e2f2978c12c3da36a79ebcff");
    transport->close();
    EXPECT_EQ(ret.port, 9093);
  }
}

/**
 * Check that we can write contents to a file and read them back correctly.
 */
TEST(chord_test, write_file) {
  auto socket = make_shared<TSocket>("10.202.6.29", 9092);
  auto transport = make_shared<TBufferedTransport>(socket);
  auto protocol = make_shared<TBinaryProtocol>(transport);
  FileStoreClient client{protocol};
  transport->open();
  {
    RFile file;
    file.__isset.meta = true;
    file.__set_content("Hello, world!");
    file.meta.__set_owner("guest");
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
