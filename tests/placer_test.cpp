#include "../src/cad/LogicBlock.hpp"
#include "../src/cad/Placer.hpp"
#include "../src/fabric/Fabric.hpp"
#include <cassert>
#include <iostream>
#include <vector>

using namespace vfpga;

void test_placer_basic() {
  std::cout << "Testing Placer Basic..." << std::endl;

  // 2x2 Fabric
  Fabric fabric(2, 2);

  // Create 2 connected blocks
  // Block 0: Output "net_1"
  // Block 1: Input "net_1"

  std::vector<LogicBlock> blocks;
  blocks.emplace_back(0, "blk0");
  blocks.back().output_net = "net_1";

  blocks.emplace_back(1, "blk1");
  blocks.back().input_nets.push_back("net_1");

  auto placement = Placer::place(fabric, blocks);

  assert(placement.size() == 2);
  assert(placement.count(0));
  assert(placement.count(1));

  auto pos0 = placement[0];
  auto pos1 = placement[1];

  std::cout << "Block 0 at (" << pos0.first << ", " << pos0.second << ")"
            << std::endl;
  std::cout << "Block 1 at (" << pos1.first << ", " << pos1.second << ")"
            << std::endl;

  // Check bounds
  assert(pos0.first >= 0 && pos0.first < 2);
  assert(pos0.second >= 0 && pos0.second < 2);
  assert(pos1.first >= 0 && pos1.first < 2);
  assert(pos1.second >= 0 && pos1.second < 2);

  // Check overlap
  assert(pos0 != pos1);

  std::cout << "Placer Basic Tests Passed!" << std::endl;
}

int main() {
  test_placer_basic();
  return 0;
}
