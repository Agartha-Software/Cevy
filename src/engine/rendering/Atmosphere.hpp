/*
** Agartha-Software, 2023
** C++evy
** File description:
** Atmosphere
*/

#pragma once

#include "Color.hpp"

namespace cevy::engine {
class Atmosphere {
  public:
  Color ambiant;
  Color fog;
  float fog_distance;

  Atmosphere(const Color &fog = {.01, 0.0225, 0.04}, float fog_distance = 500,
             const Color &ambiant = {.01, 0.0225, 0.04});
};
} // namespace cevy::engine
