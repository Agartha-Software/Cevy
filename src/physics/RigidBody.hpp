/*
** EPITECH PROJECT, 2025
** Cevy
** File description:
** Rigidbody physics
*/

#pragma once

#include "Velocity.hpp"
#include "collision/Collider.hpp"
#include <cmath>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

namespace cevy::physics {
class RigidBody {
    friend class Gravity;
    friend class PhysicsPlugin;
    public:
    // Collider collider;
    // glm::vec3 center;
    // float resititution = 1;
    float resititution = 0.95;
    // float resititution = 0.7071; // sqrt(0.5);
    // private:
    float iMass = 1; /// inverse of kilogram mass : kg⁻¹
    public:
    glm::vec3 acceleration = {0, 0, 0}; /// : (m/s²)ds : additive m/s;
    // bool animated = false;
    // bool passive = false;

    float mass() const {
      return 1 / this->iMass;
    }
    void setMass(float mass) {
      this->iMass = 1 / mass;
    }

    // template<typename ...S>
    // RigidBody(float mass, S ...shapes) : collider(std::forward<S>(shapes)...), iMass(1/mass) {};

    RigidBody(float mass) : iMass(1/mass) {};

    static RigidBody Passive() {
      RigidBody body(INFINITY);
      // body.passive = true;
      return body;
    }

    /**
     * @brief compute collisions between two bodies
     * friction ignored
     * rotation and center of mass ignored
     * @param a first body
     * @param b second body
     * @param relative_velocity : m/s
     * @return Collision with energy exchanged : m*kg
     */
    static Collision collide(const Collider &a, const glm::mat4 &tm_a, const Collider &b, const glm::mat4 &tm_b, const glm::vec3 &relative_velocity) {
      auto collisions = Collider::collide(a, tm_a, b, tm_b);

      if (collisions.size() == 0) {
        return Collision::NoHit();
      }

      glm::vec3 accumulate = {};
      glm::vec3 location = {};
      float intersection = 0;

      for (const auto &collision : collisions) {
        accumulate += collision.direction;
        location += collision.location;
        intersection += collision.intersection;
      }
      accumulate /= collisions.size();
      location /= collisions.size();
      intersection /= collisions.size();

      auto energy_ = glm::dot(accumulate, relative_velocity * 2.f);
      if (energy_ <= 0) {
        return Collision::NoHit();
      }
      auto energy = accumulate * energy_;
      return {energy, location, intersection, true};
    }

    protected:
    static void system(ecs::Query<ecs::Entity, RigidBody, const Collider, engine::Transform, option<engine::TransformVelocity>> query);
  };
}
