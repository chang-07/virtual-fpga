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
  // Route the design. Returns true if successful.
  // implementation details: Modifies the fabric to store routing info (e.g.
  // switch settings)
  bool route(Fabric &fabric, const std::vector<LogicBlock> &blocks,
             const std::map<int, std::pair<int, int>> &placement);

  // Store nets for analysis
  struct Net {
    struct Point {
      int x, y;
    };
    Point source;
    std::vector<Point> sinks;
    std::vector<Point> path; // Full routing path (list of nodes/tiles)
  };
  std::vector<Net> nets; // Instance member, but route is static...
  // We should probably make Router a class instance, or just return nets from
  // route? Or make nets static? Static is messy. The TimingAnalyzer takes
  // 'const Router&'. So Router was intended to be an instance. But
  // Router::route is static.

private:
  // Build the routing graph based on Fabric resources
  void build_graph(Fabric &fabric, std::vector<RoutingNode> &graph);

  // Find path for a single net using A*
  std::vector<int> route_net(const std::vector<RoutingNode> &graph,
                             int start_node, int end_node);
};

} // namespace vfpga
