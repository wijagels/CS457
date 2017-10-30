#include "message.hpp"
#include "common.hpp"
#include "proto/bank.pb.h"
#include <cstring>

BranchMessage &BranchNetMsg::msg() noexcept { return d_msg; }

std::array<unsigned char, BranchNetMsg::header_length> &BranchNetMsg::header() noexcept { return d_header; }

size_t &BranchNetMsg::body_size() noexcept { return d_body_size; }

void BranchNetMsg::decode_header() noexcept {
  std::memcpy(&d_body_size, d_header.data(), sizeof(d_body_size));
  d_body_size = ntohll(d_body_size);
}

void BranchNetMsg::encode_header() noexcept {
  size_t size_net = htonll(d_body_size);
  std::memcpy(d_header.data(), &size_net, sizeof(size_net));
}
