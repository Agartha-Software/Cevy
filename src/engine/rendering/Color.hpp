/*
** Agartha-Software, 2023
** C++evy
** File description:
** Color
*/

#pragma once

namespace cevy::engine {
class Color {
  public:
  unsigned char r; // Color red value
  unsigned char g; // Color green value
  unsigned char b; // Color blue value
  unsigned char a; // Color alpha value

  Color(unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255);
  // operator ::Color();
  // operator const ::Color() const;
};
} // namespace cevy::engine
