#pragma once

#include "../core/LogicVal.hpp"
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace vfpga {

template <size_t K> class LUT {
public:
  LUT() {
    // Initialize with 0s
    config_mask.resize(1 << K, LogicState::L0);
  }

  // Configure the LUT with a bitmask
  // Mask is a vector of LogicVals, size must be 2^K
  void configure(const std::vector<LogicVal> &mask) {
    if (mask.size() != (1 << K)) {
      throw std::invalid_argument("Invalid mask size for LUT");
    }
    config_mask = mask;
  }

  // Lookup
  LogicVal evaluate(const std::vector<LogicVal> &inputs) const {
    if (inputs.size() != K) {
      throw std::invalid_argument("Invalid number of inputs for LUT");
    }

    size_t index = 0;
    for (size_t i = 0; i < K; ++i) {
      if (inputs[i].is_X() || inputs[i].is_Z()) {
        // If any input is unknown, the output is likely unknown
        // (Unless all possible outputs for this X are the same, but let's keep
        // it simple for now)
        return LogicState::LX;
      }
      if (inputs[i].is_1()) {
        index |= (1 << i);
      }
    }

    return config_mask[index];
  }

private:
  std::vector<LogicVal> config_mask;
};

} // namespace vfpga
