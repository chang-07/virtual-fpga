#pragma once

#include "../core/LogicVal.hpp"
#include <cmath>
#include <vector>

namespace vfpga {

class BRAM {
public:
  int depth;
  int width;
  std::vector<std::vector<LogicVal>> memory;

  // Ports
  // For simplicity, we'll use member variables for ports or methods?
  // Methods are better for behavioral model.

  BRAM(int d = 1024, int w = 8) : depth(d), width(w) {
    memory.resize(depth,
                  std::vector<LogicVal>(width, LogicVal(LogicState::L0)));
  }

  // Synchronous Write
  void write(int address, const std::vector<LogicVal> &data_in,
             LogicVal write_enable) {
    if (write_enable.is_1()) {
      if (address >= 0 && address < depth) {
        memory[address] = data_in;
      }
    }
  }

  // Asynchronous Read (or Sync? Real BRAMs are usually sync read, but for
  // simplicity async read is easier first) Let's go with Async Read for now,
  // update to Sync if needed for DFF timing. Actually, "Synchronous read/write"
  // was in the plan. So we need an internal read_data register.

  std::vector<LogicVal> read(int address) const {
    if (address >= 0 && address < depth) {
      return memory[address];
    }
    return std::vector<LogicVal>(width, LogicVal(LogicState::LX));
  }

  // In a real cycle-based step:
  // step(clk, addr, din, we) -> dout
};

} // namespace vfpga
