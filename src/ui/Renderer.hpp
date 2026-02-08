#pragma once

#include "../fabric/Fabric.hpp"
#include <string>

namespace vfpga {

class Renderer {
public:
  Renderer(int width, int height, const std::string &title);
  ~Renderer();

  // Check if window should close
  bool should_close();

  // Main draw loop
  void draw(const Fabric &fabric);

private:
  int window_width;
  int window_height;

  // Drawing helpers
  void draw_grid(const Fabric &fabric);
  // void draw_wires(...)
};

} // namespace vfpga
