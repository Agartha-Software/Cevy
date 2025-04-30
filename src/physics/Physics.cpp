/*
** EPITECH PROJECT, 2023
** R-Type
** File description:
** physics.cpp
*/

#include "Physics.hpp"
#include "App.hpp"
#include "Spring.hpp"
#include "Transform.hpp"
#include <chrono>
#include <glm/ext/quaternion_geometric.hpp>

void cevy::physics::PhysicsPlugin::build(cevy::ecs::App &app) {
  app.init_component<cevy::physics::RigidBody>();
  app.init_component<cevy::physics::Collider>();
  app.init_component<cevy::physics::Spring>();
  app.init_resource<cevy::physics::Gravity>();
  app.init_resource<cevy::physics::RigidBodyWorld>();
  app.add_systems<core_stage::PreUpdate>(Gravity::system);
  app.add_systems<core_stage::PostUpdate>(RigidBody::system);
  app.add_systems<core_stage::PostUpdate>(RigidBodyWorld::system);
  app.add_systems<core_stage::PostUpdate>(Spring::system);
}

void cevy::physics::Gravity::system(Query<RigidBody> query, Resource<Time> time,
                                    Resource<Gravity> gravity) {
  for (auto [body] : query) {
    if (body.iMass != 0) {
      body.acceleration += gravity->acceleration * float(time->delta_seconds());
    }
  }
}

/**
 * @brief
 * performed in external frame
 * @param query
 */
void cevy::physics::RigidBody::system(
    ecs::Query<Entity, RigidBody, const Collider, engine::Transform,
               option<engine::TransformVelocity>>
        query) {
  for (auto [a, body_a, collider_a, transform_a, motion_a] : query) {
    if (!collider_a.layers)
      continue;
    for (auto [b, body_b, collider_b, transform_b, motion_b] : query) {
      if (a >= b || (body_a.iMass == 0 && body_b.iMass == 0) ||
          !(collider_a.layers & collider_b.layers))
        continue;
      auto vel_a = motion_a.has_value() ? motion_a->position : glm::vec3(0, 0, 0);
      auto vel_b = motion_b.has_value() ? motion_b->position : glm::vec3(0, 0, 0);
      auto vel_d = vel_b - vel_a;

      const glm::mat4 &tm_a = transform_a;
      const glm::mat4 &tm_b = transform_b;

      auto energy = RigidBody::collide(collider_a, tm_a, collider_b, tm_b, vel_d);
      if (!energy.hit) {
        continue;
      }
      assert(energy.direction != glm::vec3(0, 0, 0));

      float conservation_a = body_a.iMass / (body_a.iMass + body_b.iMass);
      float conservation_b = body_b.iMass / (body_a.iMass + body_b.iMass);

      float restitution = body_a.resititution * body_b.resititution;
      transform_a.position +=
          (conservation_a)*glm::normalize(energy.direction) * energy.intersection;
      transform_b.position -=
          (conservation_b)*glm::normalize(energy.direction) * energy.intersection;
      if (motion_a) {
        motion_a->position += energy.direction * (restitution * conservation_a);
      };
      if (motion_b) {
        motion_b->position -= energy.direction * (restitution * conservation_b);
      };
    }
  }
}

void cevy::physics::RigidBodyWorld::system(
    Query<RigidBody, engine::TransformVelocity, engine::Transform, option<Collider>> query,
    Resource<Time> time, Resource<RigidBodyWorld> world) {
  if (world->dragDensity != 0) {
    for (auto [body, velocity, transform, o_collider] : query) {
      if (body.iMass != 0 && o_collider) {

        auto v = velocity.position;
        auto _v_ = glm::length(v);
        auto k = o_collider->dragCoefficient * world->dragDensity * o_collider->area *
                 glm::dot(transform.scale, transform.scale);
        auto i_m = body.iMass;

        auto dv_dt = v * _v_ * k * i_m;

        if (!velocity.animated) {
          velocity.position /= 1 + k * i_m * _v_ * float(time->delta_seconds());
        }



        // body.acceleration -= velocity.position * glm::length(velocity.position) *
        //                      world->dragDensity * o_collider->dragCoefficient * o_collider->area *
        //                      glm::dot(transform.scale, transform.scale) *
        //                      float(time->delta_seconds()) /* * float(time->delta_seconds())  */ *
        //                      body.iMass / 2.f;
      }
    }
  }
  for (auto [body, motion, transform, _collider] : query) {
    if (!motion.animated) {
      motion.position += body.acceleration;
    }
    body.acceleration = glm::vec3(0);
  }
}
