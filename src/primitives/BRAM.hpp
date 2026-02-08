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

  static constexpr int DELAY_READ_PS = 1000; // 1ns read delay

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

  std::vector<LogicVal> read(int address) const {
    if (address >= 0 && address < depth) {
      return memory[address];
    }
    return std::vector<LogicVal>(width, LogicVal(LogicState::LX));
  }
};

} // namespace vfpga
