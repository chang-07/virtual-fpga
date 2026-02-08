#pragma once

#include "LogicVal.hpp"
#include <vector>

namespace vfpga {

class Signal {
public:
  Signal() : value(LogicState::LX) {}
  explicit Signal(LogicVal val) : value(val) {}

  // Get current resolved value
  LogicVal get() const { return value; }

  // Drive the signal (simple assignment for now, future: multiple drivers)
  void drive(LogicVal val) { value = val; }

  // Contention resolution (simplified for now)
  // In a real simulation, we might accumulate drivers and resolve at the end of
  // a cycle
  void resolve(const std::vector<LogicVal> &drivers);

private:
  LogicVal value;
};

} // namespace vfpga
