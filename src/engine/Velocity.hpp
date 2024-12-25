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

class TransformVelocity : public engine::Transform {
  public:
  TransformVelocity() : engine::Transform(){};
  TransformVelocity(const Transform &tm) : engine::Transform(tm){};
  ~TransformVelocity(){};

  /// delta scale
  TransformVelocity &operator*=(float s) {
    invalidate();

    rotation = glm::normalize(glm::slerp(glm::quat(), rotation, std::max(0.f, s)));
    position *= s;

    return *this;
  }

  /// delta scale
  TransformVelocity operator*(float s) {
    TransformVelocity ret = *this;
    ret *= s;
    return ret;
  }

  static void
  system(ecs::Query<engine::Transform, TransformVelocity, option<cevy::engine::PhysicsProps>> q,
         ecs::Resource<cevy::ecs::Time> time) {
    float delta_t = time.get().delta_seconds();
    for (auto [tm, vel, phys] : q) {
      float decay = 0.995;
      if (phys.has_value())
        decay = 1 - phys.value().decay;
      tm *= vel * delta_t;
      vel *= powf(decay, delta_t);
    }
  }

  protected:
  private:
};
} // namespace cevy::engine
