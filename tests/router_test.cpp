#include "../src/cad/LogicBlock.hpp"
#include "../src/cad/Router.hpp"
#include "../src/fabric/Fabric.hpp"
#include <cassert>
#include <iostream>
#include <map>
#include <vector>

using namespace vfpga;

void test_router_basic() {
  std::cout << "Testing Router Basic..." << std::endl;

  // 3x3 Fabric to allow some routing space
  Fabric fabric(3, 3);

  // Placement:
  // Block 0 at (0,0) -> Output "net_1"
  // Block 1 at (2,2) -> Input "net_1"

  std::vector<LogicBlock> blocks;
  blocks.emplace_back(0, "blk0");
  blocks.back().output_net = "net_1";

  blocks.emplace_back(1, "blk1");
  blocks.back().input_nets.push_back("net_1");

  std::map<int, std::pair<int, int>> placement;
  placement[0] = {0, 0};
  placement[1] = {2, 2};

  bool success = Router::route(fabric, blocks, placement);
  assert(success);

  std::cout << "Router Basic Tests Passed!" << std::endl;
}

int main() {
  test_router_basic();
  return 0;
}
