#include "snapshot.hpp"

Snapshot::CompletedState::CompletedState(const RecordingState &rs) : d_transferred{rs.transferred} {}

uint64_t Snapshot::CompletedState::transferred() const { return d_transferred; }

Snapshot::Snapshot(size_t n, uint64_t id, uint64_t balance)
    : d_states{n}, d_id{id}, d_balance{balance} {}

uint64_t Snapshot::id() { return d_id; }

void Snapshot::record_tx(const std::string &from, uint64_t amount) {
  auto &s = d_states.at(from);
  if (s.type() == typeid(RecordingState)) {
    boost::get<RecordingState>(s).transferred += amount;
  }  // else no-op
}

void Snapshot::start_recording(const std::string &from) {
  auto &s = d_states.at(from);
  boost::get<EmptyState>(s);  // Enforce previous state
  s = RecordingState{};
}

void Snapshot::stop_recording(const std::string &from) {
  auto &s = d_states.at(from);
  s = CompletedState{boost::get<RecordingState>(s)};
}

ReturnSnapshot Snapshot::to_message() noexcept(false) {
  ReturnSnapshot snap;
  auto local_snap = snap.mutable_local_snapshot();
  local_snap->set_snapshot_id(d_id);
  local_snap->set_balance(d_balance);
  for (const auto & [ name, s ] : d_states) {
    auto state = boost::get<CompletedState>(s);  // Will explode if too soon
    auto ch_state = local_snap->add_channel_state();
    ch_state->set_name(name);
    ch_state->set_in_transit(state.transferred());
  }
  return snap;
}
