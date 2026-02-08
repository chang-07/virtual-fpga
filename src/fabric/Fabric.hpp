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

  Fabric(int w, int h);

  struct Point {
    int x, y;
  };
  struct Connectivity {
    Point source;
    std::vector<Point> sinks;
  };
  std::vector<Connectivity> nets;

  // Accessors
  Tile &get_tile(int x, int y);
  const Tile &get_tile(int x, int y) const;

  size_t size() const { return grid.size(); }

  // Simulation Control
  void step();  // Advance clock
  void reset(); // Reset all DFFs

  // IO Interaction
  void set_input(int x, int y, LogicVal value);
  LogicVal get_output(int x, int y) const;
};

} // namespace vfpga
