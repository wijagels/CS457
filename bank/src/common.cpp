#include "common.hpp"
extern "C" {
#include <endian.h>
}

size_t htonll(size_t value) noexcept { return htobe64(value); }

size_t ntohll(size_t value) noexcept { return be64toh(value); }
