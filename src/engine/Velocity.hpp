/*
** EPITECH PROJECT, 2024
** rtype
** File description:
** Velocity
*/

#pragma once

#include "PhysicsProps.hpp"
#include "Query.hpp"
#include "Resource.hpp"
#include "Time.hpp"
#include "Transform.hpp"
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>

namespace cevy {
namespace engine {

inline static glm::vec3 lerp(const glm::vec3 &A, const glm::vec3 &B, float t) {
  return A * t + B * (1.f - t);
}

class TransformVelocity : public engine::Transform {
  public:
  bool animated = false;
  TransformVelocity() : engine::Transform() {};
  TransformVelocity(const Transform &tm) : engine::Transform(tm) {};
  ~TransformVelocity() {};

  /// delta scale
  TransformVelocity &operator*=(float s) {

    position *= s;
    rotation = glm::slerp(glm::identity<glm::quat>(), rotation, s);
    scale = glm::pow(scale, glm::vec3(s, s, s));

    return *this;
  }

  /// delta scale
  TransformVelocity operator*(float s) {
    TransformVelocity ret = *this;
    ret *= s;
    return ret;
  }

  protected:
  template <typename Windower>
  friend class Engine;
  static void
  system(ecs::Query<engine::Transform, TransformVelocity, option<cevy::engine::PhysicsProps>> q,
         ecs::Resource<cevy::ecs::Time> time) {
    float delta_t = time->delta_seconds();
    for (auto [tm, vel, phys] : q) {
      if (vel.animated)
        continue;
      auto scaled = vel * delta_t;
      tm.position += scaled.position;
      tm.rotation = glm::normalize(scaled.rotation * tm.rotation);
      tm.scale *= scaled.scale;
      float decay = 1;
      if (phys.has_value()) {
        decay = 1 - phys.value().decay;
        vel *= powf(decay, delta_t);
      }
    }
  }

  private:
};
} // namespace engine
template <>
inline std::string
reflect<cevy::engine::TransformVelocity>(const cevy::engine::TransformVelocity &t) {
  return reflect<engine::TransformVelocity>() + "::{\n" + "linear=" + reflect(t.position) + "\n" +
         "rotational=" + reflect(t.rotation) + "\n" + "scaling=" + reflect(t.scale) + "\n" + "}";
};
} // namespace cevy
