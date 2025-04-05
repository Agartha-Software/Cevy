/*
** Agartha-Software, 2024
** C++evy
** File description:
** Directional spot light
*/

#pragma once

#include <glm/glm.hpp>

namespace cevy::engine {
class SpotLight {
  public:
  glm::vec3 color;
  float softness = 0.1;
  float angle = 3.14;
  float range = 32;
  uint16_t viewlayer_bits;
  uint16_t shadow_viewlayer_bits;
};

class SunLight {
  public:
  glm::vec3 color;
  float range = 32;
  float radius = 32;
  uint16_t viewlayer_bits;
  uint16_t shadow_viewlayer_bits;
};
} // namespace cevy::engine
