#include "channel.hpp"
#include "proto/bank.pb.h"
#include <zmq.hpp>

void channel::send_branch_msg(const BranchMessage &msg) {
  auto size = static_cast<size_t>(msg.ByteSize());
  zmq::message_t zmsg{size};
  msg.SerializeToArray(zmsg.data(), size);
}
