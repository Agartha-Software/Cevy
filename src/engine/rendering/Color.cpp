/*
** Agartha-Software, 2023
** C++evy
** File description:
** Color
*/

#include "Color.hpp"

cevy::engine::Color::Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

const glm::vec4 &cevy::engine::Color::as_vec() const {
  return *reinterpret_cast<const glm::vec4 *>(this);
}

const glm::vec3 &cevy::engine::Color::as_vec3() const {
  return *reinterpret_cast<const glm::vec3 *>(this);
}

cevy::engine::Color::operator const glm::vec4 &() const { return this->as_vec(); }

cevy::engine::Color::operator const glm::vec3 &() const { return this->as_vec3(); }

// cevy::engine::Color::operator glm::vec4 &&() {
//   return std::move(*reinterpret_cast<glm::vec4 *>(this));
//   ;
// }

// cevy::engine::Color::operator glm::vec3 &&() {
//   return std::move(*reinterpret_cast<glm::vec3 *>(this));
//   ;
// }
