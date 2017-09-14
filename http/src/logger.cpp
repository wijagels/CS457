#include "logger.hpp"

#include <atomic>
#include <mutex>
#include <ostream>
#include <sstream>
#include <stdexcept>

Logger::Logger(std::ostream &os) : d_output_stream{os} {}

void Logger::log_get(const std::string &path, const std::string &address, uint16_t client_port) {
  // I can't be bothered to look up this return type
  decltype(++d_counts.at(std::string{})) count;
  try {
    count = ++d_counts.at(path);
  } catch (std::out_of_range) {
    {
      std::lock_guard<std::mutex> lg(d_write_lock);
      d_counts.emplace(path, 0);
    }  // Release lock asap
    count = ++d_counts.at(path);
  }
  // Prevent interleaving/locking by using a temporary stream
  std::ostringstream os;
  os << path << "|" << address << "|" << client_port << "|" << count << "\n";
  d_output_stream << os.str();
}
