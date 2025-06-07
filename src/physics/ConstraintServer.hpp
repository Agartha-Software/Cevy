/*
** EPITECH PROJECT, 2025
** Cevy
** File description:
** Collision iterated solver
*/

#pragma once

#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include <unordered_map>

#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/matrix.hpp>

#include "Entity.hpp"
#include "RigidBody.hpp"
#include "Motion.hpp"
#include "collision/Collider.hpp"

namespace cevy::physics {
struct Constraint {
  ecs::Entity a;
  ecs::Entity b;
  glm::vec3 direction;
  float intersection;
};
struct PartialSolve {
  Constraint constraint;
  // glm::vec3 acceleration;
  float correction_a;
  float correction_b;
};
struct BodyConstraint {
  glm::vec3 correction {0};
  glm::vec3 load {0};
  glm::vec3 acceleration {0};
  float undershoot = 1.f;
  bool overconstrained = false;
};
class ConstraintServer {
  std::vector<Constraint> constraints;

  public:
  void addConstraint(Constraint &&constraint) {
    this->constraints.push_back(std::forward<Constraint>(constraint));
  }

  static void system(ecs::Resource<ConstraintServer> self,
                     ecs::Query<RigidBody, engine::Transform, option<engine::Motion>> query) {
    self->solve(query);
  }

  void solve(ecs::Query<RigidBody, engine::Transform, option<engine::Motion>> &query) {

    std::unordered_map<ecs::Entity, BodyConstraint> bodies;
    std::vector<PartialSolve> solves;
    solves.reserve(constraints.size());

    for (const auto &constraint : constraints) {
      auto correction = std::max(0.f, constraint.intersection) * constraint.direction;
      auto got_a = query.get(constraint.a);
      auto got_b = query.get(constraint.b);
      if (!got_a || !got_b)
        continue;
      auto [body_a, _ta, va] = got_a.value();
      auto [body_b, _tb, vb] = got_b.value();
      float part_a = body_a.iMass / (body_a.iMass + body_b.iMass);
      float part_b = body_b.iMass / (body_a.iMass + body_b.iMass);

      std::cout << "corrections:" << part_a << ", " << part_b << ", * " << cevy::reflect(correction)
      << std::endl;
      std::cout << "accelerations:" << cevy::reflect(body_a.acceleration) << ", "
      << cevy::reflect(body_b.acceleration) << std::endl;
      bodies.emplace(constraint.a, BodyConstraint{ .acceleration = body_a.acceleration});
      bodies.emplace(constraint.b, BodyConstraint{ .acceleration = body_b.acceleration});
      bodies[constraint.a].correction += part_a * correction;
      bodies[constraint.b].correction -= part_b * correction;

      if (constraint.intersection >= 0) {
        if (va && vb) {
          auto dv = va->linear - vb->linear;
          auto v1 = glm::dot(dv, constraint.direction);
          if (v1 < 0) {
            va->linear -= v1 * 1 * part_a * constraint.direction;
            vb->linear += v1 * 1 * part_b * constraint.direction;
          }
        }
      }

      solves.push_back({.constraint = constraint,
                        // .acceleration = body_a.acceleration - body_b.acceleration,
                        .correction_a = part_a,
                        .correction_b = part_b});
    }
    for (const auto &solve : solves) {
      auto &constraint = solve.constraint;
      auto got_a = query.get(constraint.a);
      auto got_b = query.get(constraint.b);
      if (!got_a || !got_b)
        continue;
      auto [body_a, _ta, motion_a] = got_a.value();
      auto [body_b, _tb, motion_b] = got_b.value();

      auto &constraint_a = bodies[constraint.a];
      auto improvement_a = glm::dot(constraint.direction, constraint_a.correction);
      auto undershoot_a = improvement_a / solve.correction_a;

      constraint_a.undershoot = std::min(constraint_a.undershoot, undershoot_a);
      constraint_a.overconstrained |= improvement_a < 0;

      auto &constraint_b = bodies[constraint.b];
      auto improvement_b = glm::dot(constraint.direction, -constraint_b.correction);
      auto undershoot_b = improvement_b / solve.correction_b;

      constraint_b.undershoot = std::min(constraint_b.undershoot, undershoot_b);
      constraint_b.overconstrained |= improvement_b < 0;

      {
        auto loading_share_a = std::max(0.f, glm::dot(constraint_a.correction, constraint.direction)) /
                              glm::length(constraint_a.correction);
        auto loading_a =
            std::max(0.f, loading_share_a * glm::dot(constraint.direction, constraint_b.acceleration));
        auto loading_share_b = std::max(0.f, glm::dot(-constraint_b.correction, constraint.direction)) /
                              glm::length(constraint_b.correction);
        auto loading_b =
            std::max(0.0f, loading_share_b * glm::dot(constraint.direction, -constraint_a.acceleration));

        std::cout << "loading_share:" << loading_share_a << ", " << loading_share_b << std::endl;
        std::cout << "acc:" << cevy::reflect(constraint_a.acceleration) << ", " << cevy::reflect(constraint_b.acceleration) << std::endl;
        std::cout << "direction:" << cevy::reflect(constraint.direction) << std::endl;
        std::cout << "loading:" << loading_a << ", " << loading_b << std::endl;

        constraint_a.load += std::isnan(loading_share_a) ? glm::vec3(0) : (loading_share_a) * constraint.direction;
        constraint_b.load += std::isnan(loading_share_b) ? glm::vec3(0) : (loading_share_b) * -constraint.direction;
      }
      // {
      //   auto loading_share_a = std::max(0.f, glm::dot(constraint_a.correction, constraint.direction)) /
      //     glm::length(constraint_a.correction);
      //   auto loading_share_b = std::max(0.f, glm::dot(-constraint_b.correction, constraint.direction)) /
      //     glm::length(constraint_b.correction);
      //   float conservation_a = body_a.iMass / (body_a.iMass + body_b.iMass);
      //   float conservation_b = body_b.iMass / (body_a.iMass + body_b.iMass);

      //   auto vel_a = motion_a.has_value() ? motion_a->position : glm::vec3(0, 0, 0);
      //   auto vel_b = motion_b.has_value() ? motion_b->position : glm::vec3(0, 0, 0);
      //   auto vel_d = vel_b - vel_a;

      //   if (glm::dot(vel_d, constraint.direction) <= 0) {
      //     continue;
      //   }
      //   if (motion_a) {
      //     motion_a->position += constraint.direction * conservation_a * (vel_d * 1.f);
      //     // if (!std::isnan(loading_share_a)) constraint_a.load += constraint.direction * loading_share_a * conservation_a * (vel_d * 1.f);
      //   };
      //   if (motion_b) {
      //     motion_b->position -= constraint.direction * conservation_b * (vel_d * 1.f);
      //     // if (!std::isnan(loading_share_b)) constraint_b.load -= constraint.direction * loading_share_b * conservation_b * (vel_d * 1.f);
      //   };
      // }
    }
    for (auto &[ent, body] : bodies) {
      auto got = query.get(ent);
      if (!got)
        continue;
      auto [rbod, tm, v] = got.value();
      // if (!body.overconstrained) {
        tm.position += body.correction;
        // tm.position += body.correction / (body.undershoot / 2 + 1);
        // std::cout << "undershoot " << size_t(ent) << " :" << body.undershoot << std::endl;

      // }
      if (rbod.iMass) {
        rbod.acceleration += body.load;
      }
      std::cout << "loading body " << size_t(ent) << " :" << cevy::reflect(body.load) << std::endl;
    }

    this->constraints.clear();
  }
};

} // namespace cevy::physics
