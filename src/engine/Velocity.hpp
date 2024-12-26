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

namespace cevy::engine {
class Velocity : public glm::vec3 {
  public:
  Velocity(){};
  ~Velocity(){};

  protected:
  private:
};

inline static glm::vec3 lerp( const glm::vec3& A, const glm::vec3& B, float t ){
  return A*t + B*(1.f-t) ;
}


class TransformVelocity : public engine::Transform {
  public:
  TransformVelocity() : engine::Transform(){};
  TransformVelocity(const Transform &tm) : engine::Transform(tm){};
  ~TransformVelocity(){};

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
  friend class Engine;
  static void
  system(ecs::Query<engine::Transform, TransformVelocity, option<cevy::engine::PhysicsProps>> q,
         ecs::Resource<cevy::ecs::Time> time) {
    float delta_t = time.get().delta_seconds();
    for (auto [tm, vel, phys] : q) {
      tm *= vel * delta_t;
      float decay = 1;
      if (phys.has_value()) {
        decay = 1 - phys.value().decay;
        vel *= powf(decay, delta_t);
      }
    }
  }

  private:
};
} // namespace cevy::engine
