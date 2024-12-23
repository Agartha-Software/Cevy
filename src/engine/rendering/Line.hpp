/*
** Agartha-Software, 2023
** Cevy
** File description:
** Line
*/

#pragma once

#include <glm/glm.hpp>

namespace cevy::engine {
class Line {
  public:
  glm::vec3 start;
  glm::vec3 end;

  Line(glm::vec3 start, glm::vec3 end) : start(start), end(end) {}
  // operator Ray() { return Ray{.position = start, .direction = end}; }
};
} // namespace cevy::engine
