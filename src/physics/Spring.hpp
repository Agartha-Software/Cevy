/*
** EPITECH PROJECT, 2025
** Cevy
** File description:
** Spring relations
*/

#pragma once

#include "Entity.hpp"
#include "Query.hpp"
#include "RigidBody.hpp"
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>

namespace cevy::physics {
struct Spring {
  ecs::Entity head;
  ecs::Entity tail;
  float restLength;
  float springRate;
  float dampingRate;

  static void
  system(ecs::Resource<cevy::ecs::Time> time,
         ecs::Query<ecs::Entity, RigidBody, const engine::Transform, const engine::Motion> bodies,
         ecs::Query<Spring, option<engine::Transform>> springs) {
    for (auto [spring, o_transform] : springs) {
      auto o_a = bodies.get(spring.head);
      auto o_b = bodies.get(spring.tail);
      if (!o_a || !o_b)
        continue;
      auto [en_a, body_a, transform_a, velocity_a] = o_a.value();
      auto [en_b, body_b, transform_b, velocity_b] = o_b.value();

      float dt = time->delta_seconds();
      glm::vec3 difference = transform_b.get_world().position - transform_a.get_world().position;
      float length = glm::length(difference);
      float force = std::min(std::max(0.f, length - spring.restLength), spring.restLength) * spring.springRate * dt;
      difference /= length;

      glm::vec3 directedForce = difference * force;
      glm::vec3 dv = velocity_a.linear - velocity_b.linear;
      dv += directedForce / 2.f;

      glm::vec3 dampedVelocity = dv * 0.1f + difference * glm::dot(difference, dv);

      directedForce -= dampedVelocity * std::min(2.f, spring.dampingRate * dt);

      float conservation_a = body_a.iMass / (body_a.iMass + body_b.iMass);
      float conservation_b = body_b.iMass / (body_a.iMass + body_b.iMass);

      body_a.acceleration += directedForce * conservation_a * body_a.iMass;
      body_b.acceleration -= directedForce * conservation_b * body_b.iMass;

      if (o_transform) {
        o_transform->position = transform_a.position;
        o_transform->rotation = glm::quatLookAt(
            -difference, glm::normalize(o_transform->rotation * glm::vec3(0, 1, 0)));
        o_transform->scale = {1 / std::sqrt(length), 1 / std::sqrt(length), length};
      }
    }
  }
};
} // namespace cevy::physics
