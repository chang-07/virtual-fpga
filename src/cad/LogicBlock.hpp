#pragma once

#include "../primitives/LUT.hpp"
#include <map>
#include <string>
#include <vector>

namespace vfpga {

struct LogicBlock {
  int id; // Unique ID for placement solver
  std::string name;

  // Resources used
  bool use_lut;
  bool use_dff;

  // LUT Configuration (if used)
  std::vector<LogicVal> lut_mask;

  // Connectivity
  // Map port names (LUT inputs, DFF input, Output) to Net names
  std::vector<std::string> input_nets; // e.g. ["net_1", "net_2", ...]
  std::string output_net;              // e.g. "net_3"
  std::string clock_net;

  LogicBlock(int _id, std::string _name)
      : id(_id), name(_name), use_lut(false), use_dff(false) {}
};

} // namespace vfpga
