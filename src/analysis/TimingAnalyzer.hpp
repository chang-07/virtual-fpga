#pragma once

#include "../cad/Router.hpp"
#include "../fabric/Fabric.hpp"
#include <map>
#include <string>
#include <vector>

namespace vfpga {

struct TimingNode {
  int x, y;
  // Enum for Pin Type?
  // Simplified: Just use tile coord and assume we track output pins.
  // Or we need a graph of logic elements.
  // Let's simplified global graph:
  // Nodes = Tile Outputs
  // Edges = Routing + Logic Delay

  // Actually, to get critical path, we need to trace signals.
  // Router has 'nets'. A net connects Source -> Sinks.
  // Source has a CLK-to-Q delay (if DFF) or Combinational Delay (if LUT).
  // Sink typically goes into LUT input or DFF input.

  // Arrival Time at Source Pin
  // Arrival Time at Sink Pin = Source Arrival + Routing Delay
  // Arrival Time at Logic Output = Sink Arrival (max of inputs) + Logic Delay
};

struct TimingResult {
  double fmax_mhz;
  double critical_path_delay_ns;
  std::vector<std::pair<int, int>> critical_path_nodes;
};

class TimingAnalyzer {
public:
  const Fabric &fabric;
  const Router &router;

  TimingAnalyzer(const Fabric &f, const Router &r) : fabric(f), router(r) {}

  TimingResult analyze();
};

} // namespace vfpga
