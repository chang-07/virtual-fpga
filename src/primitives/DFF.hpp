#pragma once

#include "../core/LogicVal.hpp"

namespace vfpga {

class DFF {
public:
  static constexpr int DELAY_SETUP_PS = 50;  // 50ps setup
  static constexpr int DELAY_CLK_Q_PS = 100; // 100ps clk-to-q

  DFF() : state(LogicState::LX), next_state(LogicState::LX) {}

  // Prepare next state (combinational phase)
  void props(LogicVal d_in, LogicVal enable = LogicState::L1,
             LogicVal reset = LogicState::L0) {
    // Async reset check (if we wanted async reset, but let's stick to sync for
    // now as per plan logic) Actually, plan said "synchronous state element".

    // For simulation, we usually capture D at the end of the comb phase
    if (reset.is_1()) {
      next_state = LogicState::L0;
    } else if (enable.is_1()) {
      if (d_in.is_Z())
        next_state = LogicState::LX; // Z into DFF is X
      else
        next_state = d_in;
    } else {
      next_state = state; // Hold state
    }
  }

  // Commit state (clock edge)
  void update() { state = next_state; }

  LogicVal get_output() const { return state; }
  void set_state(LogicVal s) { state = s; }

private:
  LogicVal state;
  LogicVal next_state;
};

} // namespace vfpga
