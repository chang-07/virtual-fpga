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
  std::vector<std::pair<int, int>> free_tiles;

  // 1. Random Initialization
  std::vector<std::pair<int, int>> all_tiles;
  for (const auto &tile : fabric.grid) {
    all_tiles.push_back({tile.x, tile.y});
  }

  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(all_tiles.begin(), all_tiles.end(), g);

  for (size_t i = 0; i < blocks.size(); ++i) {
    current_locs[blocks[i].id] = all_tiles[i];
  }
  for (size_t i = blocks.size(); i < all_tiles.size(); ++i) {
    free_tiles.push_back(all_tiles[i]);
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
      // Propose a move:
      // 1. Move a block to an empty tile
      // 2. Swap two blocks

      // For simplicity, let's pick a random block and move it to a random new
      // location (either empty or occupied)

      if (blocks.empty())
        break;

      std::uniform_int_distribution<> distr_block(0, blocks.size() - 1);
      int idx = distr_block(g);
      int block_id = blocks[idx].id;

      // Pick a destination tile
      // It could be from free_tiles OR from current_locs (swap)
      // Let's just pick any tile in the fabric
      std::uniform_int_distribution<> distr_x(0, fabric.width - 1);
      std::uniform_int_distribution<> distr_y(0, fabric.height - 1);
      int new_x = distr_x(g);
      int new_y = distr_y(g);

      std::pair<int, int> old_pos = current_locs[block_id];
      std::pair<int, int> new_pos = {new_x, new_y};

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

      // Calculate delta cost locally? Or just full recalculate for simplicity
      // (slower but safer) Let's do full recalculate first Temporary apply move
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
    // Optionally print progress
    // std::cout << "Temp: " << temp << " Cost: " << current_cost << std::endl;
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
