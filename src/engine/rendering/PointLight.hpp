/*
** Agartha-Software, 2024
** C++evy
** File description:
** Omnidirectional point light
*/

#pragma once

#include <glm/glm.hpp>

namespace cevy::engine {
class PointLight {
  public:
  glm::vec3 color;
  float radius;
  float range;
  uint16_t viewlayer_bits;
  uint16_t shadow_viewlayer_bits;
};
} // namespace cevy::engine
