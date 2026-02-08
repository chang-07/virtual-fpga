#pragma once

#include "../fabric/Fabric.hpp"
#include "LogicBlock.hpp"
#include <map>
#include <set>
#include <vector>

namespace vfpga {

// Represents a node in the routing graph
struct RoutingNode {
  int id; // Unique ID
  int x, y;
  // Type of node? (WIRE, PIN)
  // For simplicity, let's just say everything is a node.

  // Edges
  std::vector<int> neighbors;

  // Pathfinder state
  double base_cost = 1.0;
  double cur_congestion_cost = 0.0;
  double hist_congestion_cost = 0.0;
  int occupancy = 0;
  int capacity = 1;
};

class Router {
public:
  // Route the design. Returns true if successful.
  // implementation details: Modifies the fabric to store routing info (e.g.
  // switch settings)
  static bool route(Fabric &fabric, const std::vector<LogicBlock> &blocks,
                    const std::map<int, std::pair<int, int>> &placement);

private:
  // Build the routing graph based on Fabric resources
  static void build_graph(Fabric &fabric, std::vector<RoutingNode> &graph);

  // Find path for a single net using A*
  static std::vector<int> route_net(const std::vector<RoutingNode> &graph,
                                    int start_node, int end_node);
};

} // namespace vfpga
