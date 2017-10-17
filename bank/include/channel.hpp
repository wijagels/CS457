#include "proto/bank.pb.h"
#include <string>
#include <zmq.hpp>

class channel {
 public:
  channel(const std::string &endpoint) : d_context{1}, d_socket{d_context, ZMQ_REQ} {
    d_socket.connect(endpoint);
  }

  void send_branch_msg(const BranchMessage &msg);

 private:
  zmq::context_t d_context;
  zmq::socket_t d_socket;
};
