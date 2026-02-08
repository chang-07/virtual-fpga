#pragma once

#include "../fabric/Fabric.hpp"
#include "LogicBlock.hpp"
#include <map>
#include <random>
#include <vector>

namespace vfpga {

class Placer {
public:
  struct Placement {
    int tile_x;
    int tile_y;
    // Which logic block is here? -1 if empty
    int block_id;
  };

  // Main entry point
  // Returns mapping: BlockID -> (x, y)
  static std::map<int, std::pair<int, int>>
  place(Fabric &fabric, const std::vector<LogicBlock> &blocks);

private:
  // Helper to calculate cost (HPWL)
  static double
  calculate_cost(const std::vector<LogicBlock> &blocks,
                 const std::map<int, std::pair<int, int>> &locations);

  // Helper to calculate HPWL for a single net
  static int get_net_hpwl(const std::vector<std::pair<int, int>> &points);
};

} // namespace vfpga
