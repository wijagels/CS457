#pragma once
#include <cstring>
#include <google/protobuf/message.h>
#include <type_traits>
extern "C" {
#include <endian.h>
}

namespace kvstore::messaging {
template <typename M, typename = std::enable_if_t<std::is_base_of_v<google::protobuf::Message, M>>>
class Message {
 public:
  Message() = default;

  enum { header_length = 8 };

  std::array<unsigned char, header_length> &header() noexcept { return m_header; }

  std::size_t body_size() const noexcept { return m_body_size; }

  void set_body_size(std::size_t new_size) noexcept {
    m_body_size = new_size;
    encode_header();
  }

  void decode_header() noexcept {
    std::memcpy(&m_body_size, m_header.data(), sizeof(m_body_size));
    m_body_size = ntohll(m_body_size);
  }

  void encode_header() noexcept {
    std::size_t size_net = htonll(m_body_size);
    std::memcpy(m_header.data(), &size_net, sizeof(size_net));
  }

 private:
  std::size_t htonll(std::size_t value) noexcept { return htobe64(value); }

  std::size_t ntohll(std::size_t value) noexcept { return be64toh(value); }

  std::size_t m_body_size = 0;
  std::array<unsigned char, header_length> m_header{};
};
}  // namespace kvstore::messaging
