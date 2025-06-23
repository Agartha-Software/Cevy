/*
** EPITECH PROJECT, 2025
** Cevy
** File description:
** physics systems implementations
*/

#define GLM_FORCE_SWIZZLE

#include <glm/ext/quaternion_geometric.hpp>
#include <glm/geometric.hpp>

#include "App.hpp"
#include "ConstraintServer.hpp"
#include "Constraints.hpp"
#include "Physics.hpp"
#include "Spring.hpp"
#include "Transform.hpp"
#include "engine.hpp"

void cevy::physics::PhysicsPlugin::build(cevy::ecs::App &app) {
  app.init_component<cevy::physics::RigidBody>();
  app.init_component<cevy::physics::Collider>();
  app.init_component<cevy::physics::Spring>();
  app.init_component<cevy::physics::Rope>();
  app.init_resource<cevy::physics::Gravity>();
  app.init_resource<cevy::physics::RigidBodyWorld>();
  app.init_resource<cevy::physics::ConstraintServer>();
  app.add_systems<core_stage::PreUpdate>(Gravity::system);
  app.add_systems<core_stage::PostUpdate>(RigidBody::system);
  app.add_systems<core_stage::PostUpdate>(Spring::system);
  app.add_systems<core_stage::PostUpdate>(Rope::system);
  app.add_systems<core_stage::PostUpdate>(ConstraintServer::system);
  app.add_systems<core_stage::PostUpdate>(RigidBodyWorld::system);
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
    ecs::Query<Entity, RigidBody, const Collider, engine::Transform, option<engine::Motion>>
        query) {
  for (auto [a, body_a, collider_a, transform_a, motion_a] : query) {
    if (!collider_a.layers)
      continue;
    for (auto [b, body_b, collider_b, transform_b, motion_b] : query) {
      if (a >= b || (body_a.iMass == 0 && body_b.iMass == 0) ||
          !(collider_a.layers & collider_b.layers) || (!motion_a && !motion_b))
        continue;
      const engine::Motion &vel_a = motion_a.has_value() ? *motion_a : engine::Motion();
      const engine::Motion &vel_b = motion_b.has_value() ? *motion_b : engine::Motion();

      const glm::mat4 &tm_a = transform_a;
      const glm::mat4 &tm_b = transform_b;

      auto collisions = RigidBody::collide(collider_a, tm_a, collider_b, tm_b);
      for (const auto &collision : collisions) {
        glm::vec3 pos_a = collision.location - transform_a.position;
        glm::vec3 pos_b = collision.location - transform_b.position;

        auto vel_a_local =
            vel_a.linear +
            (vel_a.angular != glm::vec3() ? glm::cross(vel_a.angular, pos_a) : glm::vec3());
        auto vel_b_local =
            vel_b.linear +
            (vel_b.angular != glm::vec3() ? glm::cross(vel_b.angular, pos_b) : glm::vec3());

        auto relative_v = vel_b_local - vel_a_local;

        auto impulse = glm::dot(collision.direction, relative_v * 2.f);

        auto friction = relative_v - collision.direction * impulse / 2.f;

        if (glm::dot(friction, friction) != 0) {
          friction = glm::normalize(friction) * std::min(.02f, glm::length(friction));
        }

        if (impulse <= 0)
          continue;

        float conservation_a = body_a.iMass / (body_a.iMass + body_b.iMass);
        float conservation_b = body_b.iMass / (body_a.iMass + body_b.iMass);

        float restitution = body_a.resititution * body_b.resititution;

        glm::vec3 impulse_a =
            collision.direction * impulse * conservation_a * restitution + friction;
        glm::vec3 impulse_b =
            -collision.direction * impulse * conservation_b * restitution - friction;

        glm::vec3 push_a = collision.direction * collision.intersection * conservation_a;
        glm::vec3 push_b = -collision.direction * collision.intersection * conservation_b;

        transform_a += body_a.impulse(transform_a, push_a, pos_a);
        transform_b += body_b.impulse(transform_b, push_b, pos_b);

        if (motion_a && !motion_a->animated) {
          motion_a.value() += body_a.impulse(transform_a, impulse_a, pos_a);
        };
        if (motion_b && !motion_b->animated) {
          motion_b.value() += body_b.impulse(transform_b, impulse_b, pos_b);
        };
      }
    }
  }
}

void cevy::physics::RigidBodyWorld::system(
    Query<RigidBody, option<engine::Motion>, engine::Transform, option<Collider>> query,
    Resource<Time> time, Resource<RigidBodyWorld> world) {
  for (auto [body, motion, transform, _collider] : query) {
    if (motion && !motion->animated) {
      transform.position += 0.5f * body.acceleration * float(time->delta_seconds());
      motion->linear += body.acceleration;
    }
    body.acceleration = glm::vec3(0);
  }
  if (world->dragDensity != 0) {
    for (auto [body, o_velocity, transform, o_collider] : query) {
      if (body.iMass != 0 && o_collider && o_velocity) {

        auto v = o_velocity->linear;
        auto _v_ = glm::length(v);
        auto k = o_collider->dragCoefficient * world->dragDensity * o_collider->area *
                 glm::dot(transform.scale, transform.scale);
        auto i_m = body.iMass;

        auto dv_dt = v * _v_ * k * i_m;

        auto kw = o_collider->angularDragCoefficient * world->dragDensity * o_collider->area *
                  glm::dot(transform.scale, transform.scale * transform.scale);

        if (!o_velocity->animated) {
          o_velocity->linear /= 1 + 2 * k * i_m * _v_ * float(time->delta_seconds());
          o_velocity->angular /=
              1 + 2 * kw * i_m * glm::length(o_velocity->angular) * float(time->delta_seconds());
        }
      }
    }
  }
}
