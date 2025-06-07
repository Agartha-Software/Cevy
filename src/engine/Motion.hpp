/*
** EPITECH PROJECT, 2025
** Cevy
** File description:
** Motion
*/

#pragma once

#define GLM_FORCE_SWIZZLE
#include <iostream>

#include <glm/geometric.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <glm/trigonometric.hpp>

#include "PhysicsProps.hpp"
#include "Query.hpp"
#include "Resource.hpp"
#include "Time.hpp"
#include "Transform.hpp"

namespace cevy::engine {
  namespace detail {
    template <typename R, typename T> R signum(T val) {
      return (T(0) < val) - (val < T(0));
    }
  }
class Motion {
  public:
  glm::vec3 linear = {0, 0, 0};
  glm::vec4 angular = {0, 0, 1, 0};
  bool animated;

  Motion(glm::vec3 linear = {}, glm::vec4 angular = {0, 0, 1, 0}, bool animated = false) {
    this->linear = linear;
    this->angular = angular;
    this->animated = animated;
  }

  /// delta scale
  Motion &operator*=(float s) {

    this->linear *= s;
    this->angular.w *= s;
    // rotation = glm::slerp(glm::identity<glm::quat>(), rotation, s);
    // scale = glm::pow(scale, glm::vec3(s, s, s));

    return *this;
  }

  Motion &operator*=(glm::quat q) {
    const glm::vec4 b = {glm::axis(q), glm::angle(q)};
    const glm::vec4 &a = this->angular;
    glm::vec4 new_angular;
    new_angular.w = glm::acos(glm::cos(a.w) * glm::cos(b.w) -
                              glm::dot(a.xyz() * glm::sin(a.w), b.xyz() * glm::sin(b.w)));
    new_angular.xyz() = glm::cos(a.xyz()) * b.w * glm::sin(b.xyz()) +
                        glm::cos(b.xyz()) * a.w * glm::sin(a.xyz()) +
                        glm::cross(a.w * glm::sin(a.xyz()), b.w * glm::sin(b.xyz()));
    new_angular.xyz() = glm::normalize(new_angular.xyz());

    this->angular = new_angular;
    return *this;
  }

  /// delta scale
  Motion operator*(float s) {
    Motion ret = *this;
    ret *= s;
    return ret;
  }

  void composeAngular(glm::vec3 axis, float angle) {
    const glm::vec4 &b = {axis, angle / 2.f};
    const glm::vec4 &a = {this->angular.xyz(), this->angular.w / 2.f};
    glm::vec4 new_angular;
    new_angular.w = glm::acos(glm::cos(a.w) * glm::cos(b.w) -
                              glm::dot(a.xyz() * glm::sin(a.w), b.xyz() * glm::sin(b.w)));
    // std::cout << "ANGLE" << cevy::reflect(new_angular.w) << std::endl;
    new_angular = {glm::cos(a.xyz()) * b.xyz() * glm::sin(b.w) +
                       glm::cos(b.xyz()) * a.xyz() * glm::sin(a.w) +
                       glm::cross(a.xyz() * glm::sin(a.w), b.xyz() * glm::sin(b.w)),
                   new_angular.w};
    // std::cout << "AXIS" << cevy::reflect(new_angular.xyz()) << std::endl;
    new_angular = {glm::normalize(new_angular.xyz()), new_angular.w};
    // std::cout << "AXIS" << cevy::reflect(new_angular.xyz()) << std::endl;

    this->angular = new_angular;
    this->angular.w *= 2;
  }

  void composeAngular2(glm::vec3 axis, float angle) {
    using glm::acos;
    using glm::asin;
    using glm::cos;
    using glm::cross;
    using glm::dot;
    using glm::sin;

    const float &alpha_2 = this->angular.w;
    const glm::vec3 &l = this->angular.xyz();

    const float &beta_2 = angle;
    const glm::vec3 &m = axis;

    float cos_gamma_2;

    glm::vec3 sin_gamma_2_n;

    cos_gamma_2 = cos(alpha_2) * cos(beta_2) - sin(alpha_2) * sin(beta_2) * dot(l, m);

    sin_gamma_2_n = sin(alpha_2) * cos(beta_2) * l + cos(alpha_2) * sin(beta_2) * m +
                    sin(alpha_2) * sin(beta_2) * cross(l, m);


    float gamma_2 = acos(cos_gamma_2);
    float sin_gamma_2 = glm::length(sin_gamma_2_n);
    glm::vec3 n = sin_gamma_2_n / sin_gamma_2;
    gamma_2 *= detail::signum<float>(asin(sin_gamma_2));
    // float gamma_2 = asin(sin_gamma_2) * detail::signum<float>(-acos(cos_gamma_2));
    // glm::vec3 n = sin_gamma_2_n / sin(gamma_2);

    this->angular = {n, gamma_2 * 2};

    // gamma = glm::acos(glm::cos(a.w) * glm::cos(b.w) -
    //                           glm::dot(a.xyz() * glm::sin(a.w), b.xyz() * glm::sin(b.w)));
    // std::cout << "ANGLE" << cevy::reflect(new_angular.w) << std::endl;
    // new_angular = glm::cos(a.xyz()) * b.w * glm::sin(b.xyz()) +
    //                    glm::cos(b.xyz()) * a.w * glm::sin(a.xyz()) +
    //                    glm::cross(a.w * glm::sin(a.xyz()), b.w * glm::sin(b.xyz())),
    //                new_angular.w};
    // std::cout << "AXIS" << cevy::reflect(new_angular.xyz()) << std::endl;
    // new_angular = {glm::normalize(new_angular.xyz()), new_angular.w };
    // std::cout << "AXIS" << cevy::reflect(new_angular.xyz()) << std::endl;

    // this->angular = new_angular;
    // this->angular.w *= 2;
  }

  void composeAngular3(glm::vec3 axis, float angle) {
      glm::vec3 out = glm::normalize(glm::cross(this->angular.xyz(), axis));
      glm::vec3 turn_a = glm::cross(this->angular.xyz(), out) * this->angular.w;
      glm::vec3 turn_b = glm::cross(axis, out) * angle;

      glm::vec3 new_axis = glm::cross(out, turn_a + turn_b);
      this->angular = {glm::normalize(new_axis), glm::length(turn_a + turn_b)};
  }

  void composeAngular4(glm::vec3 axis, float angle) {
    this->angular = {axis * angle + this->angular.xyz() * this->angular.w, 1};
    this->angular.w = glm::length(this->angular.xyz());
    this->angular = {this->angular.xyz() / this->angular.w, this->angular.w};
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
      tm.rotation =
          glm::rotate(glm::quat({0, 0, 0}), scaled.angular.w, scaled.angular.xyz()) * tm.rotation;
      // tm.rotation *= glm::rotate(glm::quat({0, 0, 0}), scaled.angular.w, scaled.angular.xyz());
      // tm.rotation = glm::rotate(tm.rotation, scaled.angular.w, scaled.angular.xyz());
      //   tm.scale *= scaled.scale;
    }
  }
};
} // namespace cevy::engine
