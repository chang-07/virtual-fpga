#include "TimingAnalyzer.hpp"
#include "../primitives/BRAM.hpp"
#include "../primitives/DFF.hpp"
#include "../primitives/DSP.hpp"
#include "../primitives/LUT.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>

namespace vfpga {

TimingResult TimingAnalyzer::analyze() {
  TimingResult result;
  result.fmax_mhz = 0.0;
  result.critical_path_delay_ns = 0.0;
  result.critical_path_nodes.clear();

  std::map<std::pair<int, int>, double> arrival_times;
  std::map<std::pair<int, int>, std::pair<int, int>> prev_node;

  // Step 1: Initialize Arrival Times (Sources)
  for (int y = 0; y < fabric.height; ++y) {
    for (int x = 0; x < fabric.width; ++x) {
      const auto &tile = fabric.get_tile(x, y);

      if (tile.type == TileType::CLB) {
        // CLB has DFF. Assume registered output.
        arrival_times[{x, y}] = DFF::DELAY_CLK_Q_PS;
      } else if (tile.type == TileType::BRAM) {
        // BRAM read delay
        arrival_times[{x, y}] = BRAM::DELAY_READ_PS;
      } else if (tile.type == TileType::DSP) {
        // DSP output delay
        arrival_times[{x, y}] = DSP::DELAY_MUL_PS;
      } else {
        arrival_times[{x, y}] = 0.0; // Input pads or others
      }
      prev_node[{x, y}] = {-1, -1}; // No predecessor
    }
  }

  // Step 2: Propagate Delays
  int max_depth = fabric.width * fabric.height;
  double max_arrival = 0;
  std::pair<int, int> worst_node = {-1, -1};

  for (int iter = 0; iter < max_depth; ++iter) {
    bool changed = false;

    for (const auto &net : router.nets) {
      int src_x = net.source.x;
      int src_y = net.source.y;

      double src_time = arrival_times[{src_x, src_y}];

      for (const auto &sink : net.sinks) {
        int dst_x = sink.x;
        int dst_y = sink.y;

        int dist = std::abs(src_x - dst_x) + std::abs(src_y - dst_y);
        double route_delay = dist * 50.0; // 50ps per hop

        // Destination Logic Delay
        double logic_delay = 0;
        const auto &dst_tile = fabric.get_tile(dst_x, dst_y);

        if (dst_tile.type == TileType::CLB) {
          logic_delay = LUT<4>::DELAY_PS;
        } else if (dst_tile.type == TileType::DSP) {
          logic_delay = DSP::DELAY_MUL_PS;
        }
        // BRAM write setup?

        double new_time = src_time + route_delay + logic_delay;

        if (new_time > arrival_times[{dst_x, dst_y}]) {
          arrival_times[{dst_x, dst_y}] = new_time;
          prev_node[{dst_x, dst_y}] = {src_x, src_y};
          changed = true;

          if (new_time > max_arrival) {
            max_arrival = new_time;
            worst_node = {dst_x, dst_y};
          }
        }
      }
    }

    if (!changed)
      break;
  }

  // Step 3: Check Setup Constraints & Backtrace
  double total_path = max_arrival + DFF::DELAY_SETUP_PS; // Add setup time
  result.critical_path_delay_ns = total_path / 1000.0;

  if (result.critical_path_delay_ns > 0) {
    result.fmax_mhz = 1000.0 / result.critical_path_delay_ns;
  } else {
    result.fmax_mhz = 0;
  }

  // Backtrace
  if (worst_node.first != -1) {
    std::pair<int, int> curr = worst_node;
    while (curr.first != -1) {
      result.critical_path_nodes.push_back(curr);
      curr = prev_node[curr];

      // Safety check for loops (though propagation algorithm is acyclic-ish)
      if (std::find(result.critical_path_nodes.begin(),
                    result.critical_path_nodes.end(),
                    curr) != result.critical_path_nodes.end()) {
        break; // Loop detected
      }
    }
    // Reverse to get Source -> Sink order
    std::reverse(result.critical_path_nodes.begin(),
                 result.critical_path_nodes.end());
  }

  return result;
}

} // namespace vfpga
