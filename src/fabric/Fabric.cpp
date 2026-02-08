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
  // 1. Magic Routing: Propagate Outputs to Inputs
  // Resize inputs for all tiles first? Or assuming fixed size?
  // Let's clear inputs first
  for (auto &tile : grid) {
    tile.inputs.clear();
  }

  LogicVal default_val = LogicState::L0;

  for (const auto &net : nets) {
    // Get Value from Source
    // Source is likely a Register Output (DFF Q, BRAM Data Out, DSP Out)
    LogicVal val = get_output(net.source.x, net.source.y);

    // Distribute to Sinks
    for (const auto &sink : net.sinks) {
      Tile &dst = get_tile(sink.x, sink.y);
      dst.inputs.push_back(val);
    }
  }

  // 2. Evaluate Combinational Logic & Setup Registers
  for (auto &tile : grid) {
    if (tile.type == TileType::CLB) {
      // Inputs -> LUT -> DFF.D
      // Assuming checking inputs[0]...
      // For simple testing: Just pass input[0] to DFF.D (Bypass LUT)
      // Or process LUT if configured.
      LogicVal d_in = LogicState::L0;
      if (!tile.inputs.empty())
        d_in = tile.inputs[0];

      // LUT Eval (TODO: Real LUT)
      // d_in = tile.lut.evaluate(tile.inputs);

      tile.dff.set_d_in(d_in);

    } else if (tile.type == TileType::BRAM) {
      // inputs -> BRAM inputs
      // BRAM::write is sync.
      // Need to set up BRAM internal signals?
      // For now, no-op or simple passthrough if we had state.
    } else if (tile.type == TileType::DSP) {
      // DSP Combinational Eval
      // LogicVal res = tile.dsp.evaluate(...);
      // tile.dff.set_d_in(res); // if registered
    }
  }

  // 3. Update Clocks (Rising Edge)
  for (auto &tile : grid) {
    tile.update_synchronous();
  }
}

void Fabric::reset() {
  for (auto &tile : grid) {
    tile.dff.props(LogicVal(LogicState::L0), LogicVal(LogicState::L0),
                   LogicVal(LogicState::L1));
    tile.dff.update();
    // Reset BRAM/DSP if needed
  }
}

// Helper to get the "Registered" output of a tile
LogicVal Fabric::get_output(int x, int y) const {
  const Tile &tile = get_tile(x, y);
  if (tile.type == TileType::CLB) {
    return tile.dff.get_output();
  } else if (tile.type == TileType::BRAM) {
    // return tile.bram.get_data_out();
    return LogicState::L0;
  } else if (tile.type == TileType::DSP) {
    // return tile.dsp.get_result();
    return LogicState::L0;
  }
  return LogicState::L0;
}

} // namespace vfpga
