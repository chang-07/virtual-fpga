#pragma once

#include "Tile.hpp"
#include <stdexcept>
#include <vector>

namespace vfpga {

class Fabric {
public:
  int width;
  int height;
  std::vector<Tile> grid;

  Fabric(int w, int h) : width(w), height(h) {
    grid.reserve(w * h);
    for (int y = 0; y < h; ++y) {
      for (int x = 0; x < w; ++x) {
        grid.emplace_back(x, y);
      }
    }
  }

  Tile &get_tile(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
      throw std::out_of_range("Tile coordinates out of bounds");
    }
    return grid[y * width + x];
  }

  const Tile &get_tile(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) {
      throw std::out_of_range("Tile coordinates out of bounds");
    }
    return grid[y * width + x];
  }

  size_t size() const { return grid.size(); }
};

} // namespace vfpga
