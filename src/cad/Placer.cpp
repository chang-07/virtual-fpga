#include "Placer.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <unordered_map>

namespace vfpga {

std::map<int, std::pair<int, int>>
Placer::place(Fabric &fabric, const std::vector<LogicBlock> &blocks) {
  if (blocks.size() > fabric.size()) {
    throw std::runtime_error("Not enough resources in Fabric to place design");
  }

  std::map<int, std::pair<int, int>> current_locs;

  // 1. Organize Fabric Tiles by Type
  std::vector<std::pair<int, int>> clb_tiles;
  std::vector<std::pair<int, int>> bram_tiles;
  std::vector<std::pair<int, int>> dsp_tiles;
  std::vector<std::pair<int, int>> io_tiles;

  for (const auto &tile : fabric.grid) {
    if (tile.type == TileType::CLB)
      clb_tiles.push_back({tile.x, tile.y});
    else if (tile.type == TileType::BRAM)
      bram_tiles.push_back({tile.x, tile.y});
    else if (tile.type == TileType::DSP)
      dsp_tiles.push_back({tile.x, tile.y});
    else if (tile.type == TileType::IO)
      io_tiles.push_back({tile.x, tile.y});
  }

  // 2. Random Initialization respecting types
  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(clb_tiles.begin(), clb_tiles.end(), g);
  std::shuffle(bram_tiles.begin(), bram_tiles.end(), g);
  std::shuffle(dsp_tiles.begin(), dsp_tiles.end(), g);

  size_t clb_idx = 0;
  size_t bram_idx = 0;
  size_t dsp_idx = 0;

  for (const auto &block : blocks) {
    if (block.type == TileType::CLB) {
      if (clb_idx >= clb_tiles.size())
        throw std::runtime_error("Not enough CLB tiles");
      current_locs[block.id] = clb_tiles[clb_idx++];
    } else if (block.type == TileType::BRAM) {
      if (bram_idx >= bram_tiles.size())
        throw std::runtime_error("Not enough BRAM tiles");
      current_locs[block.id] = bram_tiles[bram_idx++];
    } else if (block.type == TileType::DSP) {
      if (dsp_idx >= dsp_tiles.size())
        throw std::runtime_error("Not enough DSP tiles");
      current_locs[block.id] = dsp_tiles[dsp_idx++];
    } else {
      // Fallback or IO?
      // Treating others as CLB for now or throw
      if (clb_idx >= clb_tiles.size())
        throw std::runtime_error("Not enough tiles for unknown type");
      current_locs[block.id] = clb_tiles[clb_idx++];
    }
  }

  double current_cost = calculate_cost(blocks, current_locs);
  double initial_temp = 100.0 * std::sqrt(blocks.size()); // Heuristic
  double final_temp = 0.01;
  double alpha = 0.95; // Cooling rate
  int moves_per_temp = 10 * blocks.size();

  double temp = initial_temp;

  // Annealing Loop
  while (temp > final_temp) {
    for (int i = 0; i < moves_per_temp; ++i) {
      if (blocks.empty())
        break;

      std::uniform_int_distribution<> distr_block(0, blocks.size() - 1);
      int idx = distr_block(g);
      const auto &block = blocks[idx];
      int block_id = block.id;

      // Pick a destination tile COMPATIBLE with block type
      std::pair<int, int> new_pos;
      if (block.type == TileType::CLB) {
        std::uniform_int_distribution<> d(0, clb_tiles.size() - 1);
        new_pos = clb_tiles[d(g)];
      } else if (block.type == TileType::BRAM) {
        std::uniform_int_distribution<> d(0, bram_tiles.size() - 1);
        new_pos = bram_tiles[d(g)];
      } else if (block.type == TileType::DSP) {
        std::uniform_int_distribution<> d(0, dsp_tiles.size() - 1);
        new_pos = dsp_tiles[d(g)];
      } else {
        continue;
      }

      std::pair<int, int> old_pos = current_locs[block_id];
      if (old_pos == new_pos)
        continue;

      // Check if occupied
      int occupied_by = -1;
      for (auto const &[bid, loc] : current_locs) {
        if (loc == new_pos) {
          occupied_by = bid;
          break;
        }
      }

      // Apply move
      current_locs[block_id] = new_pos;
      if (occupied_by != -1) {
        current_locs[occupied_by] = old_pos; // Swap
      }

      double new_cost = calculate_cost(blocks, current_locs);
      double delta = new_cost - current_cost;

      // Metropolis Criterion
      bool accept = false;
      if (delta < 0) {
        accept = true;
      } else {
        std::uniform_real_distribution<> distr_prob(0.0, 1.0);
        if (distr_prob(g) < std::exp(-delta / temp)) {
          accept = true;
        }
      }

      if (accept) {
        current_cost = new_cost;
      } else {
        // Revert
        current_locs[block_id] = old_pos;
        if (occupied_by != -1) {
          current_locs[occupied_by] = new_pos;
        }
      }
    }
    temp *= alpha;
  }

  std::cout << "Final Placement Cost: " << current_cost << std::endl;
  return current_locs;
}

double
Placer::calculate_cost(const std::vector<LogicBlock> &blocks,
                       const std::map<int, std::pair<int, int>> &locations) {
  // Build net-to-pins map
  std::unordered_map<std::string, std::vector<std::pair<int, int>>> net_points;

  // We assume IO ports are at fixed locations or ignored for now?
  // WARNING: If we don't handle IOs, the cloud of logic might just drift
  // anywhere. Ideally, IOs assume coordinates (e.g. at perimeter). For this
  // Virtual FPGA, let's treat unset IOs as staying at (0,0) cost-wise? No, that
  // would pull everything to 0,0. Let's assume input nets originate "nowhere"
  // (pure internal cost) unless we map IOs.

  for (const auto &block : blocks) {
    std::pair<int, int> pos = locations.at(block.id);

    // Output drives a net
    if (!block.output_net.empty()) {
      net_points[block.output_net].push_back(pos);
    }

    // Inputs sink nets
    for (const auto &net : block.input_nets) {
      if (!net.empty()) {
        net_points[net].push_back(pos);
      }
    }
  }

  double total_hpwl = 0;
  for (const auto &[net_name, points] : net_points) {
    if (points.size() > 1) { // Only nets with >= 2 points have length
      total_hpwl += get_net_hpwl(points);
    }
  }

  return total_hpwl;
}

int Placer::get_net_hpwl(const std::vector<std::pair<int, int>> &points) {
  int min_x = std::numeric_limits<int>::max();
  int max_x = std::numeric_limits<int>::min();
  int min_y = std::numeric_limits<int>::max();
  int max_y = std::numeric_limits<int>::min();

  for (const auto &p : points) {
    if (p.first < min_x)
      min_x = p.first;
    if (p.first > max_x)
      max_x = p.first;
    if (p.second < min_y)
      min_y = p.second;
    if (p.second > max_y)
      max_y = p.second;
  }

  return (max_x - min_x) + (max_y - min_y);
}

} // namespace vfpga
