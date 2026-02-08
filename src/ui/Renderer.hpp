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

#include <functional>

  // ...

  // Main draw loop with UI callbacks
  void draw(const Fabric &fabric, std::function<void()> on_step = nullptr,
            std::function<void()> on_reset = nullptr);

private:
  int window_width;
  int window_height;

  // Drawing helpers
  void draw_grid(Fabric &fabric);
  // void draw_wires(...)
};

} // namespace vfpga
