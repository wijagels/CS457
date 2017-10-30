#ifndef INCLUDE_MESSAGE_HPP_
#define INCLUDE_MESSAGE_HPP_
#include "proto/bank.pb.h"

class BranchNetMsg {
 public:
  BranchNetMsg() = default;

  enum { header_length = 8 };

  BranchMessage &msg() noexcept;

  std::array<unsigned char, header_length> &header() noexcept;

  size_t &body_size() noexcept;

  void decode_header() noexcept;

  void encode_header() noexcept;

 private:
  size_t d_body_size = 0;
  std::array<unsigned char, header_length> d_header{};
  BranchMessage d_msg;
};
#endif
