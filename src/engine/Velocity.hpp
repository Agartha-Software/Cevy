/*
** EPITECH PROJECT, 2024
** rtype
** File description:
** Velocity
*/

#pragma once

#include <raylib.h>
#include <raymath.h>

#include "PhysicsProps.hpp"
#include "ecs/Query.hpp"
#include "ecs/Resource.hpp"
#include "ecs/Time.hpp"
#include "Transform.hpp"
#include "Vector.hpp"

#ifdef ____cpp_lib_interpolate
using std::lerp;
#else
namespace std {
inline constexpr float lerp(float a, float b, float x) { return a + x * (b - a); }
} // namespace std
using std::lerp;
#endif

namespace cevy::engine {
class Velocity : public Vector {
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

    rotation = QuaternionNlerp(QuaternionIdentity(), rotation, s);
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
