#include "Packer.hpp"
#include <iostream>

namespace vfpga {

std::vector<LogicBlock> Packer::pack(const Netlist &netlist) {
  std::vector<LogicBlock> blocks;
  int next_id = 0;

  for (const auto &[name, cell] : netlist.cells) {
    LogicBlock block(next_id++, name);

    if (cell->type == "$lut") {
      block.use_lut = true;
      // Parse LUT mask
      // Assuming parameter "LUT" is an integer or string bitmask
      // For simplicity, default to 0
      // Real implementation needs to decode the parameter

      // Connect inputs
      // Assuming ports A, B, C, D... or A[0], A[1]
      // We iterate cell ports and map to block inputs
      for (const auto &[port_name, port] : cell->ports) {
        if (port.direction == PortDirection::INPUT) {
          if (port.connected_net) {
            block.input_nets.push_back(port.connected_net->name);
          } else {
            block.input_nets.push_back(""); // Unconnected
          }
        } else if (port.direction == PortDirection::OUTPUT) {
          if (port.connected_net) {
            block.output_net = port.connected_net->name;
          }
        }
      }

    } else if (cell->type == "DFF") {
      block.use_dff = true;
      // D -> Input
      // Q -> Output
      // C -> Clock
      // Very simple mapping for now
      if (cell->ports.count("D") && cell->ports.at("D").connected_net) {
        block.input_nets.push_back(cell->ports.at("D").connected_net->name);
      }
      if (cell->ports.count("Q") && cell->ports.at("Q").connected_net) {
        block.output_net = cell->ports.at("Q").connected_net->name;
      }
      if (cell->ports.count("C") && cell->ports.at("C").connected_net) {
        block.clock_net = cell->ports.at("C").connected_net->name;
      }
    } else if (cell->type == "$mem" || cell->type == "BRAM") {
      block.type = TileType::BRAM;
      // Simplistic mapping:
      // ADDR -> Input[0..N]
      // DATA -> Input[N+1..M]
      // OUT -> Output
      for (const auto &[port_name, port] : cell->ports) {
        if (port.direction == PortDirection::INPUT) {
          if (port.connected_net)
            block.input_nets.push_back(port.connected_net->name);
        } else if (port.direction == PortDirection::OUTPUT) {
          if (port.connected_net)
            block.output_net = port.connected_net->name;
        }
      }
    } else if (cell->type == "$mul" || cell->type == "DSP") {
      block.type = TileType::DSP;
      for (const auto &[port_name, port] : cell->ports) {
        if (port.direction == PortDirection::INPUT) {
          if (port.connected_net)
            block.input_nets.push_back(port.connected_net->name);
        } else if (port.direction == PortDirection::OUTPUT) {
          if (port.connected_net)
            block.output_net = port.connected_net->name;
        }
      }
    }

    blocks.push_back(block);
  }

  return blocks;
}

} // namespace vfpga
