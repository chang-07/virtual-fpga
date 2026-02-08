#include "Renderer.hpp"
#include <iostream>

// Fix for raygui missing TextToFloat
#include <cstdlib>
static float TextToFloat(const char *text) {
  return (float)strtod(text, nullptr);
}

// Suppress raygui warnings
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wunused-parameter"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#pragma clang diagnostic pop

namespace vfpga {

Renderer::Renderer(int width, int height, const std::string &title)
    : window_width(width), window_height(height) {

  // Prevent Raylib from logging simplified info
  SetTraceLogLevel(LOG_WARNING);

  InitWindow(width, height, title.c_str());
  SetTargetFPS(60);
}

Renderer::~Renderer() { CloseWindow(); }

bool Renderer::should_close() { return WindowShouldClose(); }

void Renderer::draw(const Fabric &fabric, std::function<void()> on_step,
                    std::function<void()> on_reset) {
  BeginDrawing();
  ClearBackground(RAYWHITE);

  // We need to pass non-const fabric to draw_grid if we want to modify it?
  // Or we just return an action?
  // For now, let's cast away constness locally to enable the toggle hack.
  // In a real app, we'd have a separate input handling phase.
  draw_grid(const_cast<Fabric &>(fabric));

  // Draw UI Panel
  // Bottom bar
  float ui_height = 80;
  float y_start = window_height - ui_height;

  DrawRectangle(0, y_start, window_width, ui_height, LIGHTGRAY);
  DrawLine(0, y_start, window_width, y_start, GRAY);

  // Controls
  if (GuiButton({10, y_start + 10, 100, 30}, "Step Clock")) {
    if (on_step)
      on_step();
  }

  if (GuiButton({120, y_start + 10, 100, 30}, "Reset")) {
    if (on_reset)
      on_reset();
  }

  // Draw FPS
  DrawFPS(window_width - 80, 10);

  EndDrawing();
}

void Renderer::draw_grid(Fabric &fabric) {
  // Determine tile size based on window and grid size
  // Leave some padding
  int padding = 20;
  int available_w = window_width - 2 * padding;
  int available_h = window_height - 2 * padding; // - UI height?
  // Let's adjust for UI height
  available_h -= 80;

  int tile_w = available_w / fabric.width;
  int tile_h = available_h / fabric.height;

  // Keep aspect ratio square?
  int tile_size = (tile_w < tile_h) ? tile_w : tile_h;

  int start_x = (window_width - (tile_size * fabric.width)) / 2;
  int start_y = (available_h - (tile_size * fabric.height)) / 2 + padding;

  for (int y = 0; y < fabric.height; ++y) {
    for (int x = 0; x < fabric.width; ++x) {
      int px = start_x + x * tile_size;
      int py = start_y + y * tile_size;

      Rectangle rect = {(float)px, (float)py, (float)tile_size - 2,
                        (float)tile_size - 2};

      // Interaction: Toggle on Click
      if (CheckCollisionPointRec(GetMousePosition(), rect)) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          std::cout << "Clicked Tile: " << x << "," << y << std::endl;
          // Toggle input
          LogicVal current = fabric.get_output(x, y);
          LogicVal next = current.is_1() ? LogicVal(LogicState::L0)
                                         : LogicVal(LogicState::L1);
          fabric.set_input(x, y, next);
        }
      }

      // Draw Tile Background
      Color color = LIGHTGRAY;

      // Visualize active output
      LogicVal out = fabric.get_output(x, y);
      if (out.is_1())
        color = GREEN;
      else if (out.is_0())
        color = DARKGRAY; // darker

      DrawRectangleRec(rect, color);
      DrawRectangleLinesEx(rect, 1, DARKGRAY);

      // Draw LUT/DFF content (simplified)
      DrawRectangle(px + 4, py + 4, tile_size / 2 - 6, tile_size - 8,
                    BLUE); // LUT
      DrawRectangle(px + tile_size / 2, py + 4, tile_size / 2 - 4,
                    tile_size - 8, RED); // DFF
    }
  }
}

} // namespace vfpga
