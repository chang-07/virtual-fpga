#include "../src/cad/Packer.hpp"
#include "../src/cad/Parser.hpp"
#include <cassert>
#include <iostream>

using namespace vfpga;

void test_packer() {
  std::cout << "Testing Packer..." << std::endl;

  // Parse the test design
  auto netlist_opt = Parser::from_json("tests/data/test_design.json");
  if (!netlist_opt) {
    netlist_opt = Parser::from_json("../tests/data/test_design.json");
  }
  assert(netlist_opt.has_value());
  const Netlist &netlist = netlist_opt.value();

  // Pack
  std::vector<LogicBlock> blocks = Packer::pack(netlist);

  assert(blocks.size() == 2);

  bool found_lut = false;
  bool found_dff = false;

  for (const auto &block : blocks) {
    if (block.use_lut) {
      found_lut = true;
      // The LUT in test_design.json connects A to net_2 (clk)
      // But wait, in test_design.json: "A": [ 2, 2 ]
      // net[2] is "clk".
      // So input_nets should contain "net_2".

      // Note: In parsing, connections are mapped to "net_2" strings.
      bool has_net_2 = false;
      for (const auto &net : block.input_nets) {
        if (net == "net_2")
          has_net_2 = true;
      }
      assert(has_net_2);

      // Output Y -> net_3
      assert(block.output_net == "net_3");
    }

    if (block.use_dff) {
      found_dff = true;
      // DFF connects D -> net_3, Q -> net_4, C -> net_2
      bool d_is_net_3 = false;
      for (const auto &net : block.input_nets) {
        if (net == "net_3")
          d_is_net_3 = true;
      }
      assert(d_is_net_3);
      assert(block.output_net == "net_4");
      assert(block.clock_net == "net_2");
    }
  }

  assert(found_lut);
  assert(found_dff);

  std::cout << "Packer Tests Passed!" << std::endl;
}

int main() {
  test_packer();
  return 0;
}
