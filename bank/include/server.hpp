#include "proto/bank.pb.h"
#include <zmq.hpp>

class server {
 public:
  server(const std::string &bind) : d_context{1}, d_socket{d_context, ZMQ_REP} {
    d_socket.bind(bind);
  }

  auto &init_branch_handler() { return d_init_branch_handler; }

  auto &init_snapshot_handler() { return d_init_snapshot_handler; }

  auto &marker_handler() { return d_marker_handler; }

  auto &retrieve_snapshot_handler() { return d_retrieve_snapshot_handler; }

  auto &return_snapshot_handler() { return d_return_snapshot_handler; }

  auto &transfer_handler() { return d_transfer_handler; }

  void serve_requests();

 private:
  template <typename T>
  using handler = std::function<void(const T &)>;

  void handle_message(const BranchMessage &msg);

  handler<InitBranch> d_init_branch_handler;
  handler<InitSnapshot> d_init_snapshot_handler;
  handler<Marker> d_marker_handler;
  handler<RetrieveSnapshot> d_retrieve_snapshot_handler;
  handler<ReturnSnapshot> d_return_snapshot_handler;
  handler<Transfer> d_transfer_handler;

  zmq::context_t d_context;
  zmq::socket_t d_socket;
};
