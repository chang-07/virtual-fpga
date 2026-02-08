#include "fabric/Fabric.hpp"
#include "ui/Renderer.hpp"
#include <iostream>

using namespace vfpga;

int main() {
  std::cout << "Starting Virtual FPGA..." << std::endl;

  // Initialize Fabric
  Fabric fabric(10, 10);

  // Initialize Renderer
  Renderer renderer(800, 600, "Virtual FPGA - v0.1");

  // Main Loop
  while (!renderer.should_close()) {
    // Update Simulation (TODO)

    // Draw
    renderer.draw(
        fabric, [&]() { std::cout << "Step Clock!" << std::endl; },
        [&]() { std::cout << "Reset!" << std::endl; });
  }

  return 0;
}
