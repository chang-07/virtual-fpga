#pragma once

#include "../core/LogicVal.hpp"
#include <cstdint>

namespace vfpga {

class DSP {
public:
  // simple multiplier: P = A * B
  // widths: A(18), B(18), P(36)

  long long evaluate(long long a, long long b) { return a * b; }

  // In terms of LogicVal vectors:
  // std::vector<LogicVal> compute(const std::vector<LogicVal>& a, const
  // std::vector<LogicVal>& b); Implementing full bitwise multiplication with
  // LogicVal is tedious but correct. For virtual FPGA, maybe we can cheat and
  // convert to/from int if no X/Z.
};

} // namespace vfpga
