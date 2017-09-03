#ifndef DEBUG_HPP_
#define DEBUG_HPP_
#include <stdio.h>

#ifndef DEBUGPRINT
#define logd(message, ...)
#else
#define logd(message, ...)                                              \
  fprintf(stderr, "DEBUG[%s:%s:%d]: " message "\n", __FILE__, __func__, \
          __LINE__, ##__VA_ARGS__)
#endif

#define loge(message, ...)                                              \
  fprintf(stderr, "ERROR[%s:%s:%d]: " message "\n", __FILE__, __func__, \
          __LINE__, ##__VA_ARGS__)

#endif
