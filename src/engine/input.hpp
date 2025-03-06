#pragma once

#include "raylib.hpp"

namespace cevy {
class Keyboard {
  private:
  /* data */
  public:
  static bool key_pressed(int key) { return (IsKeyPressed(key)); };
  static bool key_released(int key) { return (IsKeyReleased(key)); };
  static bool key_down(int key) { return (IsKeyDown(key)); };
};
} // namespace cevy
