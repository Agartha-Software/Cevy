/*
** Agartha-Software, 2023
** C++evy
** File description:
** ClearColor
*/

#pragma once

#include "Color.hpp"

namespace cevy::engine {
class ClearColor {
  public:
  Color color;

  ClearColor(const Color &color);
  // operator ::Color();
  // operator const ::Color() const;
};
} // namespace cevy::engine
