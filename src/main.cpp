#include "analysis/TimingAnalyzer.hpp"
#include "cad/Router.hpp"
#include "fabric/BitstreamLoader.hpp"
#include "fabric/Fabric.hpp"
#include "ui/Renderer.hpp"
#include <iostream>
#include <map>
#include <vector>

using namespace vfpga;

int main() {
  std::cout << "Starting Virtual FPGA..." << std::endl;

  // 1. Initialize Fabric
  Fabric fabric(10, 10);

  // 2. Initialize Renderer
  Renderer renderer(800, 600, "Virtual FPGA - v0.1");

  // 3. Define Blocks & Placement (Hardcoded for now)
  // Let's create a simple design: LUT -> DFF
  std::vector<LogicBlock> blocks;

  // Block 0: LUT (Source) at (2,2)
  blocks.emplace_back(0, "lut0");
  blocks.back().type = TileType::CLB;
  blocks.back().output_net = "net_a";

  // Block 1: DFF (Sink) at (5,5)
  blocks.emplace_back(1, "dff0");
  blocks.back().type = TileType::CLB;
  blocks.back().input_nets.push_back("net_a");

  std::map<int, std::pair<int, int>> placement;
  placement[0] = {2, 2};
  placement[1] = {5, 5};

  // 4. Routing
  Router router;
  if (!router.route(fabric, blocks, placement)) {
    std::cerr << "Routing failed!" << std::endl;
    // return 1; // Don't exit, just show empty grid if routing fails
  }

  // 5. Timing Analysis
  TimingAnalyzer analyzer(fabric, router);
  TimingResult timing = analyzer.analyze();
  std::cout << "Timing Analysis Results:" << std::endl;
  std::cout << "  Critical Path Delay: " << timing.critical_path_delay_ns
            << " ns" << std::endl;
  std::cout << "  Fmax: " << timing.fmax_mhz << " MHz" << std::endl;

  // Main Loop
  while (!renderer.should_close()) {
    // Update Simulation (TODO)

    // Draw
    renderer.draw(
        fabric, router, timing,
        [&]() { std::cout << "Step Clock!" << std::endl; },
        [&]() { std::cout << "Reset!" << std::endl; });
  }

  return 0;
}
