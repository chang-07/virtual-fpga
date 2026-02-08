#include "../src/cad/Packer.hpp"
#include "../src/cad/Parser.hpp"
#include "../src/cad/Placer.hpp"
#include "../src/cad/Router.hpp"
#include "../src/fabric/Fabric.hpp"
#include <cassert>
#include <iostream>

using namespace vfpga;

void test_full_flow() {
  std::cout << "Testing Full CAD Flow..." << std::endl;

  // 1. Setup Fabric
  Fabric fabric(10, 10); // Plenty of space

  // 2. Parse Design
  auto netlist_opt = Parser::from_json("tests/data/test_design.json");
  if (!netlist_opt) {
    netlist_opt = Parser::from_json("../tests/data/test_design.json");
  }
  assert(netlist_opt.has_value());
  const Netlist &netlist = netlist_opt.value();
  std::cout << "[Step 1] Parsed " << netlist.cells.size() << " cells."
            << std::endl;

  // 3. Pack
  std::vector<LogicBlock> blocks = Packer::pack(netlist);
  assert(!blocks.empty());
  std::cout << "[Step 2] Packed into " << blocks.size() << " blocks."
            << std::endl;

  // 4. Place
  auto placement = Placer::place(fabric, blocks);
  assert(placement.size() == blocks.size());
  std::cout << "[Step 3] Placed " << placement.size() << " blocks."
            << std::endl;

  // 5. Route
  bool route_success = Router::route(fabric, blocks, placement);
  assert(route_success);
  std::cout << "[Step 4] Routing successful." << std::endl;

  std::cout << "Full CAD Flow Passed!" << std::endl;
}

int main() {
  test_full_flow();
  return 0;
}
