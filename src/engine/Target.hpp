/*
** Agartha-Software, 2023
** Cevy
** File description:
** target
*/

#pragma once

#include <glm/glm.hpp>
// #include "Vector.hpp"

namespace cevy::engine {
class Target : public glm::vec3 {
  // using cevy::engine::;

  public:
  void operator=(const glm::vec3 &v) {
    x = v.x;
    y = v.y;
    z = v.z;
  }
};
} // namespace cevy::engine
