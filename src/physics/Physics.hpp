/*
** EPITECH PROJECT, 2024
** R-Type
** File description:
** physics.hpp
*/

#pragma once

#include "Stage.hpp"
#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/string_cast.hpp>

#include "Plugin.hpp"
#include "Time.hpp"
#include "RigidBody.hpp"
#include "collision/Collider.hpp"
#include <glm/ext/vector_float3.hpp>
#include <vector>

namespace cevy::physics {
using namespace cevy::ecs;

class Gravity {
  friend class PhysicsPlugin;
  public:
  glm::vec3 acceleration = { 0, 0, -9.81}; /// : m/s² : N/kg
  protected:
  static void system(Query<RigidBody> query, Resource<Time> time, Resource<Gravity> gravity);
};

class RigidBodyWorld {
  friend class PhysicsPlugin;
  static void system(Query<RigidBody, engine::TransformVelocity, engine::Transform, Collider> query, Resource<Time> time, Resource<RigidBodyWorld> world);
  public:
  float dragDensity = 1.204; /// mass density of the fluid: kg/m³
};

class PhysicsPlugin : public Plugin {
  public:
  void build(App &app);
};
} // namespace cevy::physics
