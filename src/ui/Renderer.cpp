#include "Renderer.hpp"
#include "raylib.h"
#include <iostream>

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

void Renderer::draw(const Fabric &fabric) {
  BeginDrawing();
  ClearBackground(RAYWHITE);

  draw_grid(fabric);

  // Draw FPS
  DrawFPS(10, 10);

  EndDrawing();
}

void Renderer::draw_grid(const Fabric &fabric) {
  // Determine tile size based on window and grid size
  // Leave some padding
  int padding = 20;
  int available_w = window_width - 2 * padding;
  int available_h = window_height - 2 * padding;

  int tile_w = available_w / fabric.width;
  int tile_h = available_h / fabric.height;

  // Keep aspect ratio square?
  int tile_size = (tile_w < tile_h) ? tile_w : tile_h;

  int start_x = (window_width - (tile_size * fabric.width)) / 2;
  int start_y = (window_height - (tile_size * fabric.height)) / 2;

  for (int y = 0; y < fabric.height; ++y) {
    for (int x = 0; x < fabric.width; ++x) {
      int px = start_x + x * tile_size;
      int py = start_y + y * tile_size;

      // Draw Tile Background
      Color color = LIGHTGRAY;
      // Highlight random tiles for test?

      DrawRectangle(px, py, tile_size - 2, tile_size - 2, color);
      DrawRectangleLines(px, py, tile_size - 2, tile_size - 2, DARKGRAY);

      // Draw Coordinates
      // DrawText(TextFormat("%d,%d", x, y), px + 2, py + 2, 10, BLACK);
    }
  }
}

} // namespace vfpga
