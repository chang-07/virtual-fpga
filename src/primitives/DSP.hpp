#pragma once

#include "../core/LogicVal.hpp"
#include <cstdint>

namespace vfpga {

class DSP {
public:
  // simple multiplier: P = A * B
  // widths: A(18), B(18), P(36)

  static constexpr int DELAY_MUL_PS = 1500; // 1.5ns mul delay

  long long evaluate(long long a, long long b) { return a * b; }
};

} // namespace vfpga
