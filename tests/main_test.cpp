#include "../src/core/LogicVal.hpp"
#include "../src/core/Signal.hpp"
#include "../src/fabric/BitstreamLoader.hpp"
#include "../src/fabric/Fabric.hpp"
#include "../src/primitives/DFF.hpp"
#include "../src/primitives/LUT.hpp"
#include <cassert>
#include <iostream>
#include <vector>

using namespace vfpga;

void test_logic_val() {
  std::cout << "Testing LogicVal..." << std::endl;

  LogicVal v0('0');
  LogicVal v1('1');
  LogicVal vX('X');
  LogicVal vZ('Z');

  assert(v0.is_0());
  assert(v1.is_1());
  assert(vX.is_X());
  assert(vZ.is_Z());

  // NOT
  assert((~v0).is_1());
  assert((~v1).is_0());
  assert((~vX).is_X());

  // AND
  assert((v0 & v0).is_0());
  assert((v0 & v1).is_0());
  assert((v1 & v0).is_0());
  assert((v1 & v1).is_1());
  assert((v1 & vX).is_X());
  assert((v0 & vX).is_0()); // 0 AND X is 0

  // OR
  assert((v0 | v0).is_0());
  assert((v0 | v1).is_1());
  assert((v1 | v0).is_1());
  assert((v1 | v1).is_1());
  assert((v1 | vX).is_1()); // 1 OR X is 1
  assert((v0 | vX).is_X());

  // XOR
  assert((v0 ^ v0).is_0());
  assert((v0 ^ v1).is_1());
  assert((v1 ^ v0).is_1());
  assert((v1 ^ v1).is_0());
  assert((v1 ^ vX).is_X());

  std::cout << "LogicVal Tests Passed!" << std::endl;
}

void test_signal_resolution() {
  std::cout << "Testing Signal Resolution..." << std::endl;

  Signal s;

  // Single driver
  s.resolve({LogicVal('1')});
  assert(s.get().is_1());

  // Multiple same drivers
  s.resolve({LogicVal('1'), LogicVal('1')});
  assert(s.get().is_1());

  // Contention
  s.resolve({LogicVal('0'), LogicVal('1')});
  assert(s.get().is_X());

  // Z handling
  s.resolve({LogicVal('Z'), LogicVal('1')});
  assert(s.get().is_1());

  s.resolve({LogicVal('Z'), LogicVal('Z')});
  assert(s.get().is_Z());

  std::cout << "Signal Resolution Tests Passed!" << std::endl;
}

void test_lut() {
  std::cout << "Testing LUT..." << std::endl;

  // 2-input LUT (XOR gate)
  vfpga::LUT<2> lut;
  lut.configure({
      vfpga::LogicState::L0, // 00
      vfpga::LogicState::L1, // 01
      vfpga::LogicState::L1, // 10
      vfpga::LogicState::L0  // 11
  });

  assert(lut.evaluate({vfpga::LogicState::L0, vfpga::LogicState::L0}).is_0());
  assert(lut.evaluate({vfpga::LogicState::L0, vfpga::LogicState::L1}).is_1());
  assert(lut.evaluate({vfpga::LogicState::L1, vfpga::LogicState::L0}).is_1());
  assert(lut.evaluate({vfpga::LogicState::L1, vfpga::LogicState::L1}).is_0());

  std::cout << "LUT Tests Passed!" << std::endl;
}

void test_dff() {
  std::cout << "Testing DFF..." << std::endl;

  vfpga::DFF dff;

  // Initial state unknown
  assert(dff.get_output().is_X());

  // Cycle 1: D=1, Enable=1, Reset=0
  dff.props(vfpga::LogicState::L1, vfpga::LogicState::L1,
            vfpga::LogicState::L0);
  // Before update, output still X
  assert(dff.get_output().is_X());

  // Clock edge
  dff.update();
  assert(dff.get_output().is_1());

  // Cycle 2: D=0, Enable=1
  dff.props(vfpga::LogicState::L0);
  dff.update();
  assert(dff.get_output().is_0());

  // Cycle 3: Enable=0 (Hold)
  dff.props(vfpga::LogicState::L1, vfpga::LogicState::L0);
  dff.update();
  assert(dff.get_output().is_0()); // Should hold 0

  // Cycle 4: Reset=1
  dff.props(vfpga::LogicState::L1, vfpga::LogicState::L1,
            vfpga::LogicState::L1);
  dff.update();
  assert(dff.get_output().is_0());

  std::cout << "DFF Tests Passed!" << std::endl;
}

void test_fabric() {
  std::cout << "Testing Fabric..." << std::endl;

  // Create a 2x2 fabric
  vfpga::Fabric fabric(2, 2);

  assert(fabric.width == 2);
  assert(fabric.height == 2);
  assert(fabric.size() == 4);

  // Check initial state of a tile
  vfpga::Tile &t00 = fabric.get_tile(0, 0);
  assert(t00.x == 0);
  assert(t00.y == 0);
  // LUT should be 0s, DFF unknown
  assert(t00.dff.get_output().is_X());

  // Check bounds checking
  try {
    fabric.get_tile(2, 0);
    assert(false && "Should have thrown out_of_range");
  } catch (const std::out_of_range &) {
    // Expected
  }

  std::cout << "Fabric Tests Passed!" << std::endl;
}

int main() {
  test_logic_val();
  test_signal_resolution();
  test_lut();
  test_dff();
  test_fabric();
  std::cout << "All Tests Passed!" << std::endl;
  return 0;
}
