#pragma once

#include "../analysis/TimingAnalyzer.hpp"
#include "../cad/Router.hpp"
#include "../fabric/Fabric.hpp"
#include "raylib.h"
#include <functional>
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
  void draw(const Fabric &fabric, const Router &router,
            const TimingResult &timing, std::function<void()> on_step = nullptr,
            std::function<void()> on_reset = nullptr);

private:
  int window_width;
  int window_height;

  // UI State
  bool simulation_paused = true;
  float simulation_speed = 5.0f; // Steps per second
  float time_accumulator = 0.0f;

  // Drawing helpers
  void update_camera();
  void draw_grid(Fabric &fabric);
  void draw_sidebar(std::function<void()> on_step,
                    std::function<void()> on_reset);
  void draw_inspector(const Fabric &fabric);
  // void draw_wires(...)

  Camera2D camera;
};

} // namespace vfpga
