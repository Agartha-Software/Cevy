/*
** Agartha-Software, 2023
** C++evy
** File description:
** Color
*/

#pragma once

#include <glm/fwd.hpp>
#include <glm/glm.hpp>

namespace cevy::engine {
class Color {
  public:
  float r; // Color red value
  float g; // Color green value
  float b; // Color blue value
  float a; // Color alpha value

  Color(float r = 1., float g = 1., float b = 1., float a = 1.);

  const glm::vec4 &as_vec() const;

  operator const glm::vec4 &() const;
  operator const glm::vec3 &() const;
  // operator glm::vec4 &&();
  // operator glm::vec3 &&();

  private:
  const glm::vec3 &as_vec3() const;
};
} // namespace cevy::engine
