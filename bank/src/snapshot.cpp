#include "snapshot.hpp"

Snapshot::CompletedState::CompletedState(const RecordingState &rs) : transferred{rs.transferred} {}

Snapshot::Snapshot(size_t n, uint64_t id, uint64_t balance)
    : d_states{n}, c_id{id}, c_balance{balance} {}

void Snapshot::initialize(const std::vector<std::string> &peers) {
  for (const auto &e : peers) {
    d_states.emplace(e, RecordingState{});
  }
}

uint64_t Snapshot::id() { return c_id; }

void Snapshot::record_tx(const std::string &from, uint64_t amount) {
  auto &s = d_states.at(from);
  if (std::holds_alternative<RecordingState>(s)) {
    std::get<RecordingState>(s).transferred += amount;
  }  // else no-op
}

void Snapshot::marker(const std::string &from) {
  auto &s = d_states.at(from);
  s.emplace<CompletedState>(std::get<RecordingState>(s));
}

ReturnSnapshot Snapshot::to_message() {
  ReturnSnapshot snap;
  auto local_snap = snap.mutable_local_snapshot();
  local_snap->set_snapshot_id(c_id);
  local_snap->set_balance(c_balance);
  for (const auto & [ name, s ] : d_states) {
    auto state = std::get<CompletedState>(s);  // Will explode if too soon
    auto ch_state = local_snap->add_channel_state();
    ch_state->set_name(name);
    ch_state->set_in_transit(state.transferred);
  }
  return snap;
}
