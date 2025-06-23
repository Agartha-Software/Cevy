/*
** EPITECH PROJECT, 2025
** Cevy
** File description:
** Rigidbody physics
*/

#pragma once

#include <vector>

#include <cmath>
#include <glm/geometric.hpp>
#include <glm/matrix.hpp>
#include <glm/vec3.hpp>

#include "Motion.hpp"
#include "cevy.hpp"
#include "collision/Collider.hpp"

namespace cevy::physics {
class RigidBody {
  friend class Gravity;
  friend class PhysicsPlugin;

  public:
  // Collider collider;
  // glm::vec3 center;
  // float resititution = 1;
  float resititution = 0.90;
  // float resititution = 0.7071; // sqrt(0.5);
  // private:

  /// inverse of kilogram mass : kg⁻¹
  float iMass = 1;

  /// inverse of inertia tensor : rad/(m∙s⁻¹)
  glm::mat3 iInertiaTensor = {0.1};

  public:
  /// : (m/s²)ds : additive m/s as an impulse;
  glm::vec3 acceleration = {0, 0, 0};
  // bool animated = false;
  // bool passive = false;

  float mass() const { return 1 / this->iMass; }
  void setMass(float mass) { this->iMass = 1 / mass; }

  RigidBody(float mass) : iMass(1 / mass) {};

  static RigidBody Passive() {
    RigidBody body(INFINITY);
    // body.passive = true;
    return body;
  }

  /**
   * @brief enumerate collisions between two bodies
   * @param a first body
   * @param b second body
   * @param tm_a first body's transform
   * @param tm_b second body's transform
   * @return list of collisions
   */
  static std::vector<Collision> collide(const Collider &a, const glm::mat4 &tm_a, const Collider &b,
                                        const glm::mat4 &tm_b) {
    return Collider::collide(a, tm_a, b, tm_b);
  }

  /**
   * @brief Compute motion from an instantaneous acceleration to a body
   *
   * Intended to be used to compute angular motion related to a force
   *
   * @param impulse change in velocity : m/s
   * @param arm position from the center of mass : m
   * @return engine::Motion instantaneous motion
   */
  engine::Motion impulse(const engine::Transform &tm, glm::vec3 impulse, glm::vec3 arm = {}) {
    float arm_length = glm::length(arm);
    glm::vec3 arm_n = arm / arm_length;
    glm::vec3 tangeantial_impulse = impulse - arm_n * glm::dot(impulse, arm_n);

    glm::vec3 axis = glm::vec3 {0, 0, 0};
    float angle = glm::length(tangeantial_impulse) * arm_length;

    angle = std::isnan(angle) ? 0 : angle;

    if (angle > 0) {
      axis = glm::normalize(glm::cross(arm_n, tangeantial_impulse));
      // axis = glm::cross(arm, axis);
    }

    // auto i_inertia_tensor = glm::mat3(tm.rotation) * this->iInertiaTensor *
    // glm::transpose(glm::mat3(tm.rotation));
    auto i_inertia_tensor = glm::mat3(0.5);

    glm::vec3 angular = i_inertia_tensor * glm::vec3(axis * angle);
    glm::vec3 tangeantial_v = glm::cross(angular, arm);

    return engine::Motion(impulse - tangeantial_v, angular);
  }

  protected:
  static void system(
      ecs::Query<ecs::Entity, RigidBody, const Collider, engine::Transform, option<engine::Motion>>
          query);
};
} // namespace cevy::physics
