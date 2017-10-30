#ifndef INCLUDE_SNAPSHOT_HPP_
#define INCLUDE_SNAPSHOT_HPP_
#include "proto/bank.pb.h"
#include <string>
#include <unordered_map>
/* std::variant is unavailable on shitmote */
#include <boost/variant.hpp>
#include <boost/variant/get.hpp>

class Snapshot {
  struct EmptyState {};
  struct RecordingState {
    uint64_t transferred = 0;
  };
  struct CompletedState {
    explicit CompletedState(const RecordingState &rs);
    uint64_t transferred() const;

   private:
    uint64_t d_transferred;
  };

 public:
  Snapshot(size_t n, uint64_t id, uint64_t balance);

  uint64_t id();

  void record_tx(const std::string &from, uint64_t amount);

  void start_recording(const std::string &from);

  void stop_recording(const std::string &from);

  ReturnSnapshot to_message() noexcept(false);

 private:
  using varstate = boost::variant<EmptyState, RecordingState, CompletedState>;
  std::unordered_map<std::string, varstate> d_states;
  uint64_t d_id;
  uint64_t d_balance;
};

#endif
