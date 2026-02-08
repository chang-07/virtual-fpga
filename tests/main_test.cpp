#include "../src/core/LogicVal.hpp"
#include "../src/core/Signal.hpp"
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

int main() {
  test_logic_val();
  test_signal_resolution();
  std::cout << "All Tests Passed!" << std::endl;
  return 0;
}
