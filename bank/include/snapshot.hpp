#ifndef INCLUDE_SNAPSHOT_HPP_
#define INCLUDE_SNAPSHOT_HPP_
#include "proto/bank.pb.h"
#include <atomic>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

class Snapshot {
  struct EmptyState {};
  struct RecordingState {
    uint64_t transferred = 0;
  };
  struct CompletedState {
    explicit CompletedState(const RecordingState &rs);
    const uint64_t transferred;
  };

 public:
  Snapshot(size_t n, uint64_t id, uint64_t balance);

  void initialize(const std::vector<std::string> &peers);

  uint64_t id();

  void record_tx(const std::string &from, uint64_t amount);

  void marker(const std::string &from);

  ReturnSnapshot to_message();

 private:
  using varstate = std::variant<EmptyState, RecordingState, CompletedState>;
  std::unordered_map<std::string, varstate> d_states;
  const uint64_t c_id;
  const uint64_t c_balance;
};

#endif
