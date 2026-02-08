#include "Router.hpp"
#include <cmath>
#include <iostream>
#include <queue>
#include <unordered_map>

namespace vfpga {

// Simplified Graph:
// Each Tile has:
// - Input Pins (as nodes)
// - Output Pin (as node)
// - We assume "Virtual Wires" connect adjacent tiles.
// This is a placeholder because building a full routing graph is complex.

void Router::build_graph(Fabric &fabric, std::vector<RoutingNode> &graph) {
  // TODO: Implement full graph construction
  // For now, let's just assume we can magically route for the prototype's sake?
  // No, we promised Pathfinder.

  // Let's make a grid of nodes. One node per tile "Crossbar".
  graph.resize(fabric.width * fabric.height);
  for (int y = 0; y < fabric.height; ++y) {
    for (int x = 0; x < fabric.width; ++x) {
      int id = y * fabric.width + x;
      graph[id].id = id;
      graph[id].x = x;
      graph[id].y = y;

      // Connect to neighbors (NSEW)
      if (x > 0)
        graph[id].neighbors.push_back(id - 1);
      if (x < fabric.width - 1)
        graph[id].neighbors.push_back(id + 1);
      if (y > 0)
        graph[id].neighbors.push_back(id - fabric.width);
      if (y < fabric.height - 1)
        graph[id].neighbors.push_back(id + fabric.width);
    }
  }
}

bool Router::route(Fabric &fabric, const std::vector<LogicBlock> &blocks,
                   const std::map<int, std::pair<int, int>> &placement) {
  std::cout << "Starting Routing..." << std::endl;

  std::vector<RoutingNode> graph;
  build_graph(fabric, graph);

  // Map placed blocks to graph nodes
  std::map<int, int> block_to_node;
  for (const auto &[bid, pos] : placement) {
    block_to_node[bid] = pos.second * fabric.width + pos.first;
  }

  // Identify Nets
  // NetName -> SourceBlockID, [SinkBlockIDs]
  struct NetInfo {
    int source_block = -1;
    std::vector<int> sink_blocks;
  };
  std::unordered_map<std::string, NetInfo> nets;

  for (const auto &block : blocks) {
    if (!block.output_net.empty()) {
      nets[block.output_net].source_block = block.id;
    }
    for (const auto &input_net : block.input_nets) {
      if (!input_net.empty()) {
        nets[input_net].sink_blocks.push_back(block.id);
      }
    }
    // Clock?
  }

  // Route each net
  for (const auto &[net_name, info] : nets) {
    if (info.source_block == -1)
      continue; // Input from IO?

    int source_node = block_to_node[info.source_block];

    for (int sink_block : info.sink_blocks) {
      int sink_node = block_to_node[sink_block];

      // BFS/A* for path
      // For now, simple BFS
      std::queue<int> q;
      q.push(source_node);
      std::unordered_map<int, int> parent;
      parent[source_node] = -1;
      bool found = false;

      while (!q.empty()) {
        int curr = q.front();
        q.pop();

        if (curr == sink_node) {
          found = true;
          break;
        }

        for (int neighbor : graph[curr].neighbors) {
          if (parent.find(neighbor) == parent.end()) {
            parent[neighbor] = curr;
            q.push(neighbor);
          }
        }
      }

      if (found) {
        // Backtrack to trace path (placeholder for actually setting switches)
        // std::cout << "Routed " << net_name << " Source: " << source_node << "
        // Sink: " << sink_node << std::endl;
      } else {
        std::cerr << "Failed to route net: " << net_name << std::endl;
        return false;
      }
    }
  }

  std::cout << "Routing Complete (Simplified)." << std::endl;
  return true;
}

} // namespace vfpga
