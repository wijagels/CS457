#ifndef debug__H
#define debug__H
#include <cstdio>

#ifdef NDEBUG
#define logd(message, ...)
#else
#define logd(message, ...)                                                             \
  std::fprintf(stderr, "DEBUG[%s:%s:%d]: " message "\n", __FILE__, __func__, __LINE__, \
               ##__VA_ARGS__)
#endif

#define loge(message, ...)                                                             \
  std::fprintf(stderr, "ERROR[%s:%s:%d]: " message "\n", __FILE__, __func__, __LINE__, \
               ##__VA_ARGS__)

#endif
