/*
** Agartha-Software, 2023
** C++evy
** File description:
** Color
*/

#pragma once

#include "raylib.h"

namespace cevy::engine {
class Color {
  public:
  unsigned char r; // Color red value
  unsigned char g; // Color green value
  unsigned char b; // Color blue value
  unsigned char a; // Color alpha value

  Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
  operator ::Color();
  operator const ::Color() const;
};
} // namespace cevy::engine
