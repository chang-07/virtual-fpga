#pragma once

#include "LogicBlock.hpp"
#include "Netlist.hpp"
#include <vector>

namespace vfpga {

class Packer {
public:
  // Convert Netlist to a list of LogicBlocks
  // For now, simple 1-to-1 mapping:
  // - Each LUT becomes a LogicBlock (use_lut=true)
  // - Each DFF becomes a LogicBlock (use_dff=true)
  // - Future optimization: Pack LUT+DFF into single Block if connected directly
  static std::vector<LogicBlock> pack(const Netlist &netlist);
};

} // namespace vfpga
