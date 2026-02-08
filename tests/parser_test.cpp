#include "../src/cad/Parser.hpp"
#include <cassert>
#include <iostream>

using namespace vfpga;

void test_parser() {
  std::cout << "Testing Parser..." << std::endl;

  // Path might need adjustment depending on where binary runs
  // Assuming running from project root or build dir
  std::string filename = "tests/data/test_design.json";
  // Check if file exists relative to current dir, strictly for debugging
  // In CMake test, working directory is usually set.

  auto netlist_opt = Parser::from_json(filename);
  if (!netlist_opt) {
    // Try absolute path if relative fails (hack for test runner context)
    // Or just fail.
    std::cerr << "Failed to parse " << filename << std::endl;
    // fallback for build dir
    netlist_opt = Parser::from_json("../tests/data/test_design.json");
  }

  assert(netlist_opt.has_value());
  Netlist &netlist = netlist_opt.value();

  // Check inputs/outputs
  assert(!netlist.inputs.empty());
  assert(netlist.inputs[0] == "clk");
  assert(!netlist.outputs.empty());
  assert(netlist.outputs[0] == "led");

  // Check Cells
  assert(netlist.cells.size() == 2);
  assert(netlist.cells.count("$lut$top$0"));
  assert(netlist.cells.count("fd"));

  auto lut = netlist.cells["$lut$top$0"];
  assert(lut->type == "$lut");
  // Check connections
  // A -> net_2 (clk)
  // Y -> net_3 (led)
  assert(lut->ports.count("A"));
  assert(lut->ports.at("A").connected_net->name == "net_2");

  auto dff = netlist.cells["fd"];
  assert(dff->type == "DFF");
  assert(dff->ports.at("C").connected_net->name ==
         "net_2"); // Connected to same clk net

  std::cout << "Parser Tests Passed!" << std::endl;
}

int main() {
  test_parser();
  return 0;
}
