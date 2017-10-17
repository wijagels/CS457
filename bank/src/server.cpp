#include "server.hpp"
#include "proto/bank.pb.h"
#include <zmq.hpp>

void server::serve_requests() {
  for (;;) {
    zmq::message_t request;
    d_socket.recv(&request);
    BranchMessage message;
    message.ParseFromArray(request.data(), request.size());
    handle_message(message);
  }
}

void server::handle_message(const BranchMessage &msg) {
  using MsgTypes = BranchMessage::BranchMessageCase;
  MsgTypes branch_msg = msg.branch_message_case();
  switch (branch_msg) {
    case MsgTypes::kInitBranch:
      return d_init_branch_handler(msg.init_branch());
    case MsgTypes::kInitSnapshot:
      return d_init_snapshot_handler(msg.init_snapshot());
    case MsgTypes::kMarker:
      return d_marker_handler(msg.marker());
    case MsgTypes::kRetrieveSnapshot:
      return d_retrieve_snapshot_handler(msg.retrieve_snapshot());
    case MsgTypes::kReturnSnapshot:
      return d_return_snapshot_handler(msg.return_snapshot());
    case MsgTypes::kTransfer:
      return d_transfer_handler(msg.transfer());
    default:  // Also MsgTypes::BRANCH_MESSAGE_NOT_SET
      return;
  }
}
