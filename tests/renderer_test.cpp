#include "../src/analysis/TimingAnalyzer.hpp"
#include "../src/cad/Router.hpp"
#include "../src/fabric/Fabric.hpp"
#include "../src/ui/Renderer.hpp"
#include <cassert>
#include <iostream>

using namespace vfpga;

void test_renderer_smoke() {
  std::cout << "Testing Renderer Smoke..." << std::endl;

  Fabric fabric(10, 10);
  Renderer renderer(800, 600, "Test Window");
  Router router;       // Dummy
  TimingResult timing; // Dummy

  // Simulate one frame
  renderer.draw(fabric, router, timing);

  bool close = renderer.should_close();
  // assert(!close);

  std::cout << "Renderer Smoke Test Passed!" << std::endl;
}

int main() {
  test_renderer_smoke();
  return 0;
}
