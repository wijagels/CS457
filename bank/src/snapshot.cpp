#include "snapshot.hpp"
#include <mutex>
#include <string>
#include <vector>

std::mutex mtx;

Snapshot::CompletedState::CompletedState(const RecordingState &rs) : transferred{rs.transferred} {}

Snapshot::Snapshot(size_t n, uint64_t id, uint64_t balance, const std::vector<peer_info> &peers)
    : d_states{n}, c_id{id}, c_balance{balance} {
  for (const auto &e : peers) {
    d_states.emplace(std::get<0>(e), RecordingState{});
  }
}

uint64_t Snapshot::id() { return c_id; }

void Snapshot::record_tx(const std::string &from, uint64_t amount) {
  try {
    auto &s = d_states.at(from);
    if (std::holds_alternative<RecordingState>(s)) {
      std::get<RecordingState>(s).transferred += amount;
    }  // else no-op
  } catch (std::out_of_range) {
    std::cerr << "Tx " << from << std::endl;
  } catch (std::bad_variant_access) {
    std::cerr << "Tx " << from << std::endl;
    std::cerr << from << std::endl;
    std::cerr << d_states.at(from).index() << std::endl;
  }
}

void Snapshot::marker(const std::string &from) {
  try {
    auto &s = d_states.at(from);
    s.emplace<CompletedState>(std::get<RecordingState>(s));
  } catch (std::out_of_range) {
    std::cerr << "Marker " << from << std::endl;
  } catch (std::bad_variant_access) {
    std::cerr << "Marker " << from << std::endl;
    std::cerr << from << std::endl;
    std::cerr << d_states.at(from).index() << std::endl;
  }
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
