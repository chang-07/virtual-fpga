#include "../src/analysis/TimingAnalyzer.hpp"
#include "../src/cad/Router.hpp"
#include "../src/fabric/Fabric.hpp"
#include <cassert>
#include <iostream>
#include <vector>

using namespace vfpga;

void test_critical_path() {
  std::cout << "Testing Critical Path Analysis..." << std::endl;

  // 1. Setup Fabric
  Fabric fabric(5, 5);

  // 2. Creating a chain: (0,0) -> (1,1) -> (2,2)
  // Logic:
  // Node A at (0,0) (Source)
  // Node B at (1,1) (Intermediate)
  // Node C at (2,2) (Sink)

  // Configure tiles to be CLBs (default) containing LUTs
  // Routing:
  // Net 1: (0,0) -> (1,1)
  // Net 2: (1,1) -> (2,2)

  Router router;

  // Net 1
  Router::Net net1;
  net1.source = {0, 0};
  net1.sinks.push_back({1, 1});
  router.nets.push_back(net1);

  // Net 2
  Router::Net net2;
  net2.source = {1, 1};
  net2.sinks.push_back({2, 2});
  router.nets.push_back(net2);

  // 3. Analyze Timing
  TimingAnalyzer analyzer(fabric, router);
  TimingResult result = analyzer.analyze();

  std::cout << "Critical Path Delay: " << result.critical_path_delay_ns << " ns"
            << std::endl;
  std::cout << "Fmax: " << result.fmax_mhz << " MHz" << std::endl;

  // Verify Path Nodes
  std::cout << "Path Nodes: ";
  for (const auto &node : result.critical_path_nodes) {
    std::cout << "(" << node.first << "," << node.second << ") ";
  }
  std::cout << std::endl;

  // Expected Path: (0,0) -> (1,1) -> (2,2)
  assert(result.critical_path_nodes.size() == 3);
  assert(result.critical_path_nodes[0].first == 0 &&
         result.critical_path_nodes[0].second == 0);
  assert(result.critical_path_nodes[1].first == 1 &&
         result.critical_path_nodes[1].second == 1);
  assert(result.critical_path_nodes[2].first == 2 &&
         result.critical_path_nodes[2].second == 2);

  std::cout << "Critical Path Test Passed!" << std::endl;
}

int main() {
  test_critical_path();
  return 0;
}
