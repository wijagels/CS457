#ifndef LOGGER_HPP_
#define LOGGER_HPP_
#include <atomic>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <ostream>
#include <string>
#include <unordered_map>

class Logger {
 public:
  Logger(std::ostream &os = std::cout);
  void log_get(const std::string &path, const std::string &address, uint16_t client_port);

 private:
  std::unordered_map<std::string, std::atomic_int_fast32_t> d_counts;
  std::mutex d_write_lock;
  std::ostream &d_output_stream;
};

#endif
