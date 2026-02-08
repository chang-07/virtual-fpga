#pragma once

#include "../primitives/DFF.hpp"
#include "../primitives/LUT.hpp"
#include <memory>
#include <vector>

namespace vfpga {

struct Tile {
  int x, y;

  // Resources in this tile
  // For simplicity, let's assume 1 LUT + 1 DFF per tile (Basic Logic Element)
  // In a real FPGA, this would be a Cluster (CLB) containing N LEs.
  LUT<4> lut; // 4-input LUT
  DFF dff;

  // Switch Matrix (simplified)
  // We need a way to store routing configuration.
  // For now, let's just store simple mux configurations as integers.
  // e.g., input_mux[0] selects which track drives LUT input 0.
  std::vector<int> input_mux_selects;

  Tile(int x_pos, int y_pos) : x(x_pos), y(y_pos) {
    input_mux_selects.resize(4, 0); // 4 inputs for LUT
  }
};

} // namespace vfpga
