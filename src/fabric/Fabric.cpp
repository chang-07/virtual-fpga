#include "Fabric.hpp"
#include <iostream>

namespace vfpga {

Fabric::Fabric(int w, int h) : width(w), height(h) {
  grid.resize(width * height);
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      grid[y * width + x] = Tile(x, y);
      Tile &tile = grid[y * width + x];

      // Columnar Layout:
      // Col 3: BRAM
      // Col 7: DSP
      // Rest: CLB (default)
      if (x == 3) {
        tile.type = TileType::BRAM;
      } else if (x == 7) {
        tile.type = TileType::DSP;
      } else {
        tile.type = TileType::CLB;
      }
    }
  }
}

Tile &Fabric::get_tile(int x, int y) {
  if (x < 0 || x >= width || y < 0 || y >= height) {
    throw std::out_of_range("Tile coordinates out of bounds");
  }
  return grid[y * width + x];
}

const Tile &Fabric::get_tile(int x, int y) const {
  if (x < 0 || x >= width || y < 0 || y >= height) {
    throw std::out_of_range("Tile coordinates out of bounds");
  }
  return grid[y * width + x];
}

void Fabric::step() {
  // 1. Evaluate Combinational Logic (LUTs, Muxes)
  // Hack: Multiple passes to propagate
  for (int i = 0; i < 5; ++i) {
    for (auto &tile : grid) {
      // Evaluate Tile Logic
    }
  }

  // 2. Update clocks for DFFs
  for (auto &tile : grid) {
    tile.dff.update(); // Assume global clock rising edge
  }
}

void Fabric::reset() {
  for (auto &tile : grid) {
    // Reset: d_in=0, enable=0, reset=1
    tile.dff.props(LogicVal(LogicState::L0), LogicVal(LogicState::L0),
                   LogicVal(LogicState::L1));
    tile.dff.update();
  }
}

void Fabric::set_input(int x, int y, LogicVal value) {
  // Hack: Override the DFF state directly
  get_tile(x, y).dff.set_state(value);
}

LogicVal Fabric::get_output(int x, int y) const {
  return get_tile(x, y).dff.get_output();
}

} // namespace vfpga
