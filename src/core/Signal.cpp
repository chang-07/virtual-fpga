#include "Signal.hpp"

namespace vfpga {

void Signal::resolve(const std::vector<LogicVal> &drivers) {
  if (drivers.empty()) {
    value = LogicState::LZ; // Floating
    return;
  }

  LogicVal result = LogicState::LZ;

  for (const auto &d : drivers) {
    if (d.state == LogicState::LZ)
      continue; // Z doesn't drive

    if (result.state == LogicState::LZ) {
      result = d; // First non-Z driver takes precedence
    } else if (result != d) {
      result = LogicState::LX; // Contention!
      break;
    }
  }

  value = result;
}

} // namespace vfpga
