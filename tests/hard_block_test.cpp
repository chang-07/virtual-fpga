#include "../src/cad/Packer.hpp"
#include "../src/cad/Placer.hpp"
#include "../src/fabric/Fabric.hpp"
#include <cassert>
#include <iostream>
#include <vector>

using namespace vfpga;

void test_hard_block_placement() {
  std::cout << "Testing Hard Block Placement..." << std::endl;

  // 1. Setup Fabric (10x10)
  // Columns: 3=BRAM, 7=DSP
  Fabric fabric(10, 10);

  // 2. Create Netlist with BRAM and DSP
  Netlist netlist;

  // BRAM Cell
  std::shared_ptr<Cell> bram_cell = std::make_shared<Cell>("my_bram", "$mem");
  netlist.cells["my_bram"] = bram_cell;

  // DSP Cell
  std::shared_ptr<Cell> dsp_cell = std::make_shared<Cell>("my_dsp", "$mul");
  netlist.cells["my_dsp"] = dsp_cell;

  // CLB Cell
  std::shared_ptr<Cell> clb_cell = std::make_shared<Cell>("my_lut", "$lut");
  netlist.cells["my_lut"] = clb_cell;

  // 3. Pack
  Packer packer;
  std::vector<LogicBlock> blocks = packer.pack(netlist);

  assert(blocks.size() == 3);

  bool found_bram = false;
  bool found_dsp = false;
  for (const auto &b : blocks) {
    if (b.name == "my_bram") {
      assert(b.type == TileType::BRAM);
      found_bram = true;
    }
    if (b.name == "my_dsp") {
      assert(b.type == TileType::DSP);
      found_dsp = true;
    }
    if (b.name == "my_lut") {
      assert(b.type == TileType::CLB);
    }
  }
  assert(found_bram);
  assert(found_dsp);

  // 4. Place
  Placer placer;
  auto placement = placer.place(fabric, blocks);

  // 5. Verify Constraints
  for (const auto &[bid, pos] : placement) {
    // Find block type
    TileType type = TileType::CLB;
    std::string name;
    for (const auto &b : blocks) {
      if (b.id == bid) {
        type = b.type;
        name = b.name;
        break;
      }
    }

    const auto &tile = fabric.get_tile(pos.first, pos.second);
    std::cout << "Block " << name << " (" << (int)type << ") placed at ("
              << pos.first << "," << pos.second << ") Type: " << (int)tile.type
              << std::endl;

    assert(tile.type == type);

    if (type == TileType::BRAM) {
      assert(pos.first == 3); // Column 3
    } else if (type == TileType::DSP) {
      assert(pos.first == 7); // Column 7
    }
  }

  std::cout << "Hard Block Placement Test Passed!" << std::endl;
}

int main() {
  test_hard_block_placement();
  return 0;
}
