/*
** Agartha-Software, 2023
** C++evy
** File description:
** color
*/

#pragma once

#include "raylib.hpp"

namespace cevy::engine {
class color {
  public:
  unsigned char r; // color red value
  unsigned char g; // color green value
  unsigned char b; // color blue value
  unsigned char a; // color alpha value

  color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
  operator ::color();
  operator const ::color() const;
};
} // namespace cevy::engine
