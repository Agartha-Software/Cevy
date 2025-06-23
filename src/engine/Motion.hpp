/*
** EPITECH PROJECT, 2025
** Cevy
** File description:
** Motion
*/

#pragma once

#define GLM_FORCE_SWIZZLE

#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include "Query.hpp"
#include "Resource.hpp"
#include "Time.hpp"
#include "Transform.hpp"

namespace cevy::engine {
namespace detail {
template <typename R, typename T>
R signum(T val) {
  return (T(0) < val) - (val < T(0));
}
} // namespace detail
class Motion {
  public:
  glm::vec3 linear = {0, 0, 0};
  glm::vec3 angular = {0, 0, 0};
  bool animated;

  Motion(glm::vec3 linear = {}, glm::vec3 angular = {0, 0, 0}, bool animated = false) {
    this->linear = linear;
    this->angular = angular;
    this->animated = animated;
  }

  /**
   * @brief Compose this motion with an other
   *
   * @return Motion& self
   */
  Motion &operator+=(const Motion &rhs) {
    this->linear += rhs.linear;
    this->angular += rhs.angular;
    return *this;
  }

  /// delta scale
  Motion &operator*=(float s) {

    this->linear *= s;
    this->angular *= s;

    return *this;
  }

  Motion &operator*=(glm::quat q) {
    const glm::vec4 b = {glm::axis(q), glm::angle(q)};
    glm::vec4 a = {this->angular, 1};

    a.w = glm::length(this->angular);
    a = {a.xyz() / a.w, a.w};
    if (a.w == 0) {
      a = {0, 0, 1, 0};
    }

    glm::vec4 new_angular;
    new_angular.w = glm::acos(glm::cos(a.w) * glm::cos(b.w) -
                              glm::dot(a.xyz() * glm::sin(a.w), b.xyz() * glm::sin(b.w)));
    new_angular.xyz() = glm::cos(a.xyz()) * b.w * glm::sin(b.xyz()) +
                        glm::cos(b.xyz()) * a.w * glm::sin(a.xyz()) +
                        glm::cross(a.w * glm::sin(a.xyz()), b.w * glm::sin(b.xyz()));
    new_angular.xyz() = glm::normalize(new_angular.xyz());

    this->angular = new_angular.xyz() * new_angular.w;
    return *this;
  }

  /// delta scale
  Motion operator*(float s) {
    Motion ret = *this;
    ret *= s;
    return ret;
  }

  /**
   * @brief Compose this motion with angular motion
   *
   * @param axis the axis of rotaition
   * @param angle the angular velocity
   */
  void composeAngular(glm::vec3 axis, float angle) {
    this->angular = {axis * angle + this->angular};
  }

  protected:
  template <typename Windower>
  friend class Engine;
  static void system(ecs::Query<engine::Transform, Motion> q, ecs::Resource<cevy::ecs::Time> time) {
    float delta_t = time->delta_seconds();
    for (auto [tm, vel] : q) {
      if (vel.animated)
        continue;
      auto scaled = vel * delta_t;
      tm.position += scaled.linear;
      auto angle = glm::length(scaled.angular);
      if (angle > 0)
        tm.rotation =
            glm::rotate(glm::quat({0, 0, 0}), angle, scaled.angular) * tm.rotation;
    }
  }
};


// impl Transform for interdependency resolution

inline Transform &Transform::operator+=(const Motion &rhs) {
  this->position += rhs.linear;

  auto angle = glm::length(rhs.angular);
  if (angle > 0)
    this->rotation =
      glm::rotate(glm::quat({0, 0, 0}), angle, rhs.angular) * this->rotation;
  return *this;
};
} // namespace cevy::engine
