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
#include "raymath.h"
#pragma clang diagnostic pop

namespace vfpga {

Renderer::Renderer(int width, int height, const std::string &title)
    : window_width(width), window_height(height) {

  // Prevent Raylib from logging simplified info
  SetTraceLogLevel(LOG_WARNING);

  InitWindow(width, height, title.c_str());
  SetTargetFPS(60);

  // Initialize Camera
  camera.target = {0.0f, 0.0f};
  camera.offset = {0.0f, 0.0f};
  camera.rotation = 0.0f;
  camera.zoom = 1.0f;
}

Renderer::~Renderer() { CloseWindow(); }

bool Renderer::should_close() { return WindowShouldClose(); }

void Renderer::update_camera() {
  // Translate based on right mouse click
  if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) ||
      IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
    Vector2 delta = GetMouseDelta();
    delta = Vector2Scale(delta, -1.0f / camera.zoom);
    camera.target = Vector2Add(camera.target, delta);
  }

  // Zoom based on mouse wheel
  float wheel = GetMouseWheelMove();
  if (wheel != 0) {
    // Get the world point that is under the mouse
    Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

    // Set the offset to where the mouse is
    camera.offset = GetMousePosition();

    // Set the target to match, so that the camera maps the world space point
    // under the cursor to the screen space point under the cursor at any zoom
    camera.target = mouseWorldPos;

    // Zoom increment
    float scaleFactor = 1.0f + (0.25f * fabsf(wheel));
    if (wheel < 0)
      scaleFactor = 1.0f / scaleFactor;
    camera.zoom = Clamp(camera.zoom * scaleFactor, 0.125f, 64.0f);
  }
}

void Renderer::draw(const Fabric &fabric, const Router &router,
                    const TimingResult &timing, std::function<void()> on_step,
                    std::function<void()> on_reset) {
  BeginDrawing();
  ClearBackground(RAYWHITE);

  update_camera();

  BeginMode2D(camera);

  // We need to pass non-const fabric to draw_grid if we want to modify it?
  // Or we just return an action?
  // For now, let's cast away constness locally to enable the toggle hack.
  // In a real app, we'd have a separate input handling phase.
  draw_grid(const_cast<Fabric &>(fabric));

  // Draw Wires (Routing)
  int padding = 20; // Must match draw_grid
  int available_w = window_width - 2 * padding;
  int available_h = window_height - 100; // - UI height
  int tile_w = available_w / fabric.width;
  int tile_h = available_h / fabric.height;
  int tile_size = (tile_w < tile_h) ? tile_w : tile_h;
  int grid_start_x = (window_width - (tile_size * fabric.width)) / 2;
  int grid_start_y = (available_h - (tile_size * fabric.height)) / 2 + padding;

  auto get_center = [&](int tx, int ty) {
    return Vector2{
        static_cast<float>(grid_start_x + tx * tile_size + tile_size / 2),
        static_cast<float>(grid_start_y + ty * tile_size + tile_size / 2)};
  };

  // Draw all nets
  for (const auto &net : router.nets) {
    if (net.path.size() > 1) {
      for (size_t i = 0; i < net.path.size() - 1; ++i) {
        Vector2 start = get_center(net.path[i].x, net.path[i].y);
        Vector2 end = get_center(net.path[i + 1].x, net.path[i + 1].y);
        DrawLineEx(start, end, 2.0f, LIGHTGRAY);
      }
    } else if (!net.sinks.empty()) {
      // Fallback for direct connections if path is empty
      Vector2 src = get_center(net.source.x, net.source.y);
      for (const auto &sink : net.sinks) {
        Vector2 dst = get_center(sink.x, sink.y);
        DrawLineEx(src, dst, 1.0f, Fade(GRAY, 0.5f));
      }
    }
  }

  // Highlight Critical Path Nodes
  for (const auto &node : timing.critical_path_nodes) {
    int cx = node.first;
    int cy = node.second;
    int px = grid_start_x + cx * tile_size;
    int py = grid_start_y + cy * tile_size;
    DrawRectangleLinesEx(
        {(float)px, (float)py, (float)tile_size, (float)tile_size}, 3.0f, RED);
  }

  EndMode2D();

  // Draw UI Overlays
  draw_inspector(fabric);
  draw_sidebar(on_step, on_reset);

  // Auto-step logic
  if (!simulation_paused && on_step) {
    time_accumulator += GetFrameTime();
    if (time_accumulator >= (1.0f / simulation_speed)) {
      on_step();
      time_accumulator = 0.0f;
    }
  }

  DrawFPS(window_width - 80, 10);

  EndDrawing();
}

void Renderer::draw_sidebar(std::function<void()> on_step,
                            std::function<void()> on_reset) {
  float ui_height = 80;
  float y_start = window_height - ui_height;

  DrawRectangle(0, y_start, window_width, ui_height, LIGHTGRAY);
  DrawLine(0, y_start, window_width, y_start, GRAY);

  // Layout
  float x = 10;
  float y = y_start + 10;
  float btn_w = 100;
  float btn_h = 30;
  float gap = 10;

  // Single Step
  if (GuiButton({x, y, btn_w, btn_h}, "Step Clock")) {
    if (on_step) {
      on_step();
      simulation_paused = true; // Pause on manual step
    }
  }
  x += btn_w + gap;

  // Reset
  if (GuiButton({x, y, btn_w, btn_h}, "Reset")) {
    if (on_reset) {
      on_reset();
      simulation_paused = true;
    }
  }
  x += btn_w + gap;

  // Play/Pause
  const char *play_label = simulation_paused ? "Play" : "Pause";
  if (GuiButton({x, y, btn_w, btn_h}, play_label)) {
    simulation_paused = !simulation_paused;
  }
  x += btn_w + gap;

  // Speed Slider
  GuiSlider({x, y, 150, btn_h}, "Speed",
            TextFormat("%.1f Hz", simulation_speed), &simulation_speed, 1.0f,
            60.0f);
}

void Renderer::draw_inspector(const Fabric &fabric) {
  // Get coordinate under mouse (using camera)
  Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

  // Calculate grid indices
  int padding = 20; // Hardcoded in draw_grid unfortunately
  int available_w = window_width - 2 * padding;
  int available_h = window_height - 100; // - UI height
  int tile_w = available_w / fabric.width;
  int tile_h = available_h / fabric.height;
  int tile_size = (tile_w < tile_h) ? tile_w : tile_h;
  int grid_start_x = (window_width - (tile_size * fabric.width)) / 2;
  int grid_start_y = (available_h - (tile_size * fabric.height)) / 2 + padding;

  int tx = (mouseWorldPos.x - grid_start_x) / tile_size;
  int ty = (mouseWorldPos.y - grid_start_y) / tile_size;

  // Check bounds
  if (tx >= 0 && tx < fabric.width && ty >= 0 && ty < fabric.height) {
    // Valid tile hovered
    const Tile &tile = fabric.get_tile(tx, ty);
    LogicVal output = fabric.get_output(tx, ty);

    // Draw Inspector Panel (Top Right)
    float panel_w = 200;
    float panel_h = 120;
    // float panel_x = window_width - panel_w - 10;
    // float panel_y = 10;
    // Actually let's put it top left to avoid FPS
    float panel_x = 10;
    float panel_y = 10;

    DrawRectangle(panel_x, panel_y, panel_w, panel_h, Fade(SKYBLUE, 0.9f));
    DrawRectangleLines(panel_x, panel_y, panel_w, panel_h, BLUE);

    int text_x = panel_x + 10;
    int text_y = panel_y + 10;
    int line_h = 20;

    DrawText(TextFormat("Tile (%d, %d)", tx, ty), text_x, text_y, 20, DARKBLUE);
    text_y += 25;

    const char *type_str = "UNKNOWN";
    switch (tile.type) {
    case TileType::CLB:
      type_str = "CLB (Logic)";
      break;
    case TileType::BRAM:
      type_str = "BRAM (Memory)";
      break;
    case TileType::DSP:
      type_str = "DSP (Math)";
      break;
    case TileType::IO:
      type_str = "I/O Pad";
      break;
    }
    DrawText(TextFormat("Type: %s", type_str), text_x, text_y, 10, BLACK);
    text_y += line_h;

    DrawText(TextFormat("Output: %s", output.to_string().c_str()), text_x,
             text_y, 10, BLACK);
    text_y += line_h;

    // Highlight hovered tile
    int px = grid_start_x + tx * tile_size;
    int py = grid_start_y + ty * tile_size;
    DrawRectangleLinesEx(
        {(float)px, (float)py, (float)tile_size, (float)tile_size}, 2.0f,
        YELLOW);
  }
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
      // Transform mouse position to world coordinates
      Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
      if (CheckCollisionPointRec(mouseWorldPos, rect)) {
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

      // Draw Content based on Type
      Tile &tile = fabric.get_tile(x, y);
      if (tile.type == TileType::CLB) {
        // Draw LUT/DFF content (simplified)
        DrawRectangle(px + 4, py + 4, tile_size / 2 - 6, tile_size - 8,
                      BLUE); // LUT
        DrawRectangle(px + tile_size / 2, py + 4, tile_size / 2 - 4,
                      tile_size - 8, RED); // DFF
      } else if (tile.type == TileType::BRAM) {
        // Orange for BRAM
        DrawRectangle(px + 4, py + 4, tile_size - 8, tile_size - 8, ORANGE);
        DrawText("BRAM", px + 5, py + tile_size / 2 - 5, 10, BLACK);
      } else if (tile.type == TileType::DSP) {
        // Purple for DSP
        DrawRectangle(px + 4, py + 4, tile_size - 8, tile_size - 8, PURPLE);
        DrawText("DSP", px + 8, py + tile_size / 2 - 5, 10, WHITE);
      }
    }
  }
}

} // namespace vfpga
