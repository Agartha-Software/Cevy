/*
** EPITECH PROJECT, 2025
** Cevy
** File description:
** Constaints
*/

#pragma once

#include "ConstraintServer.hpp"
#include "Entity.hpp"
#include "Query.hpp"
#include "Resource.hpp"
#include "RigidBody.hpp"
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/gtc/quaternion.hpp>

namespace cevy::physics {
struct Rope {
  ecs::Entity head;
  ecs::Entity tail;
  float restLength;
  float breakStrain;


  static void system(ecs::Resource<cevy::ecs::Time> time,
                     ecs::Resource<ConstraintServer> constraint_server,
                     ecs::Query<ecs::Entity, RigidBody, engine::Transform> bodies,
                     ecs::Query<Rope, option<engine::Transform>> ropes) {
    for (auto [rope, o_transform] : ropes) {
      auto o_a = bodies.get(rope.head);
      auto o_b = bodies.get(rope.tail);
      if (!o_a || !o_b)
        continue;
      auto [en_a, body_a, transform_a] = o_a.value();
      auto [en_b, body_b, transform_b] = o_b.value();

      glm::vec3 difference = transform_b.get_world().position - transform_a.get_world().position;
      float length = glm::length(difference);
      float intersection = length - rope.restLength;
      difference /= length;

      if (length / rope.restLength > rope.breakStrain) {
        continue;
      }

      constraint_server->addConstraint({
        .a = en_a,
        .b = en_b,
        .direction = difference,
        .intersection = intersection,
      });

      if (o_transform) {
        o_transform->position = transform_a.position;
        o_transform->rotation = glm::quatLookAt(
            -difference, glm::normalize(o_transform->rotation * glm::vec3(0, 1, 0)));
        o_transform->scale = {1 / std::sqrt(length), 1 / std::sqrt(length), std::min(length, rope.restLength)};
      }
    }
  }
};
} // namespace cevy::physics
