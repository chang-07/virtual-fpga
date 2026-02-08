#include "Router.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <queue>
#include <set>
#include <unordered_map>

namespace vfpga {

void Router::build_graph(Fabric &fabric, std::vector<RoutingNode> &graph) {
  // Grid of nodes. One node per tile "Crossbar".
  graph.resize(fabric.width * fabric.height);
  for (int y = 0; y < fabric.height; ++y) {
    for (int x = 0; x < fabric.width; ++x) {
      int id = y * fabric.width + x;
      graph[id].id = id;
      graph[id].x = x;
      graph[id].y = y;
      graph[id].capacity = 1; // Wires have capacity 1

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

// Pathfinder Parameters
const int MAX_ITERATIONS = 50;
const double PRES_FAC_INIT = 0.5;
const double PRES_FAC_MULT = 1.5; // Slower growth for stability
const double HIST_FAC = 1.0;

bool Router::route(Fabric &fabric, const std::vector<LogicBlock> &blocks,
                   const std::map<int, std::pair<int, int>> &placement) {
  std::cout << "Starting Routing (Pathfinder)..." << std::endl;

  std::vector<RoutingNode> graph;
  build_graph(fabric, graph);

  // Map placed blocks to graph nodes
  std::map<int, int> block_to_node;
  for (const auto &[bid, pos] : placement) {
    block_to_node[bid] = pos.second * fabric.width + pos.first;
  }

  // Identify Nets (Internal struct for routing logic)
  struct NetInfo {
    std::string name;
    int source_node = -1;
    std::vector<int> sink_nodes;
    std::vector<int> current_path; // List of nodes used
  };
  std::vector<NetInfo> internal_nets;

  // Helper to find existing net or create
  // We can't use a lambda with auto return type easily if we want to modify the
  // vector passing references. Let's just do it inline or use index.

  for (const auto &block : blocks) {
    if (block_to_node.find(block.id) == block_to_node.end())
      continue;
    int node_id = block_to_node[block.id];

    if (!block.output_net.empty()) {
      bool found = false;
      for (auto &n : internal_nets) {
        if (n.name == block.output_net) {
          n.source_node = node_id;
          found = true;
          break;
        }
      }
      if (!found) {
        internal_nets.push_back({block.output_net, node_id, {}, {}});
      }
    }
    for (const auto &input_net : block.input_nets) {
      if (!input_net.empty()) {
        bool found = false;
        for (auto &n : internal_nets) {
          if (n.name == input_net) {
            n.sink_nodes.push_back(node_id);
            found = true;
            break;
          }
        }
        if (!found) {
          internal_nets.push_back({input_net, -1, {node_id}, {}});
        }
      }
    }
  }

  double pres_fac = PRES_FAC_INIT;

  for (int iter = 0; iter < MAX_ITERATIONS; ++iter) {
    bool congestion_free = true;

    // 1. Rip-up & Route all nets
    for (auto &net : internal_nets) {
      if (net.source_node == -1)
        continue; // Input from IO? (ignore for now)
      if (net.sink_nodes.empty())
        continue;

      // Rip-up: Decrease occupancy of current path
      std::sort(net.current_path.begin(), net.current_path.end());
      net.current_path.erase(
          std::unique(net.current_path.begin(), net.current_path.end()),
          net.current_path.end());

      for (int path_node : net.current_path) {
        if (graph[path_node].occupancy > 0)
          graph[path_node].occupancy--;
      }
      net.current_path.clear();

      // Route to all sinks (MST-like or just path-to-each)
      std::vector<int> full_net_path;

      for (int sink_node : net.sink_nodes) {
        // Dijkstra with Pathfinder Cost Function
        std::vector<double> dist(graph.size(),
                                 std::numeric_limits<double>::infinity());
        std::vector<int> parent(graph.size(), -1);
        std::priority_queue<std::pair<double, int>,
                            std::vector<std::pair<double, int>>, std::greater<>>
            pq;

        dist[net.source_node] = 0;
        pq.push({0, net.source_node});

        int found_end = -1;

        while (!pq.empty()) {
          auto [d, u] = pq.top();
          pq.pop();

          if (d > dist[u])
            continue;
          if (u == sink_node) {
            found_end = u;
            break;
          }

          for (int v : graph[u].neighbors) {
            // Calculate Pathfinder Cost
            // Cost = (b + h) * p

            double occ = static_cast<double>(graph[v].occupancy);
            double cap = static_cast<double>(graph[v].capacity);
            double congestion = std::max(0.0, occ + 1.0 - cap);
            double p_n = 1.0 + congestion * pres_fac;
            double cost_v =
                (graph[v].base_cost + graph[v].hist_congestion_cost) * p_n;

            if (dist[u] + cost_v < dist[v]) {
              dist[v] = dist[u] + cost_v;
              parent[v] = u;
              pq.push({dist[v], v});
            }
          }
        }

        if (found_end != -1) {
          // Backtrack
          int curr = found_end;
          while (curr != -1) {
            full_net_path.push_back(curr);
            curr = parent[curr];
          }
        } else {
          // std::cerr << "Failed to route part of net: " << net.name <<
          // std::endl;
        }
      }

      // Deduplicate path nodes for accurate occupancy
      std::sort(full_net_path.begin(), full_net_path.end());
      full_net_path.erase(
          std::unique(full_net_path.begin(), full_net_path.end()),
          full_net_path.end());

      // Re-apply occupancy
      for (int node_id : full_net_path) {
        graph[node_id].occupancy++;
      }
      net.current_path = full_net_path;
    }

    // 2. Update Costs & Check Congestion
    congestion_free = true;
    for (auto &node : graph) {
      if (node.occupancy > node.capacity) {
        congestion_free = false;
        node.hist_congestion_cost +=
            (node.occupancy - node.capacity) * HIST_FAC;
      }
    }

    if (congestion_free) {
      std::cout << "Routing successful at iteration " << iter << std::endl;

      // Populate public nets structure for Analysis
      this->nets.clear();
      for (const auto &inet : internal_nets) {
        if (inet.source_node == -1)
          continue;
        Net net;
        // Map node ID back to x,y
        net.source = {graph[inet.source_node].x, graph[inet.source_node].y};
        for (int sink_id : inet.sink_nodes) {
          net.sinks.push_back({graph[sink_id].x, graph[sink_id].y});
        }
        this->nets.push_back(net);
      }
      return true;
    }

    // Slowly increase pressure
    pres_fac *= PRES_FAC_MULT;
  }

  std::cerr << "Routing failed to resolve congestion after " << MAX_ITERATIONS
            << " iterations." << std::endl;
  return false;
}

std::vector<int> Router::route_net(const std::vector<RoutingNode> &graph,
                                   int start_node, int end_node) {
  // Unused helper now that logic is inline
  return {};
}

} // namespace vfpga
