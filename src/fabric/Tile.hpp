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

  Tile() : x(0), y(0) { input_mux_selects.resize(4, 0); }

  Tile(int x_pos, int y_pos) : x(x_pos), y(y_pos) {
    input_mux_selects.resize(4, 0); // 4 inputs for LUT
  }
};

} // namespace vfpga
