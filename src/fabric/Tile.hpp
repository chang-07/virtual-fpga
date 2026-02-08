#pragma once

#include "../primitives/BRAM.hpp"
#include "../primitives/DFF.hpp"
#include "../primitives/DSP.hpp"
#include "../primitives/LUT.hpp"
#include <memory>
#include <vector>

namespace vfpga {

enum class TileType { CLB, BRAM, DSP, IO };

struct Tile {
  int x, y;
  TileType type = TileType::CLB;

  // Resources (Union-like usage, though simpler to just keep all members for
  // now) CLB
  LUT<4> lut; // 4-input LUT
  DFF dff;

  // Hard Blocks (Optional)
  // We can assume if type == BRAM, we use 'bram' member.
  BRAM bram;
  DSP dsp;

  // Switch Matrix (simplified)
  // We need a way to store routing configuration.
  std::vector<int> input_mux_selects;

  // Simulation Methods
  LogicVal evaluate_combinational() {
    if (type == TileType::CLB) {
      // LUT evaluation. Need inputs.
      // For now, assuming fixed inputs from "somewhere"
      // This is tricky because LUT needs vector<LogicVal> inputs.
      // We need a way to store current input values on the tile.
      return LogicState::LX;
    } else if (type == TileType::DSP) {
      // DSP evaluation
      return dsp.evaluate(LogicState::L1, LogicState::L1); // Placeholder
    } else if (type == TileType::BRAM) {
      // BRAM read (async)
      // return bram.read(addr);
      return LogicState::LX;
    }
    return LogicState::LX;
  }

  void update_synchronous() {
    if (type == TileType::CLB) {
      dff.update();
    } else if (type == TileType::BRAM) {
      // bram.write(addr, data);
    }
  }

  // Helper to set inputs (Magic Routing)
  std::vector<LogicVal> inputs;
  void set_inputs(const std::vector<LogicVal> &new_inputs) {
    inputs = new_inputs;
  }

  Tile() : x(0), y(0) { input_mux_selects.resize(4, 0); }

  Tile(int x_pos, int y_pos) : x(x_pos), y(y_pos) {
    input_mux_selects.resize(4, 0); // 4 inputs for LUT
  }
};

} // namespace vfpga
