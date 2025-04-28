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
#include <glm/gtc/quaternion.hpp>

namespace cevy::physics {
    struct Spring {
        ecs::Entity head;
        ecs::Entity tail;
        float springRate;
        float restLength;

        static void system(ecs::Resource<cevy::ecs::Time> time, ecs::Query<ecs::Entity, RigidBody, const engine::Transform> bodies, ecs::Query<Spring, option<engine::Transform>> springs) {
            for (auto [spring, o_transform]: springs) {
                auto o_a = bodies.get(spring.head);
                auto o_b = bodies.get(spring.tail);
                if (!o_a || !o_b)
                    continue;
                auto [en_a, body_a, transform_a] = o_a.value();
                auto [en_b, body_b, transform_b] = o_b.value();

                glm::vec3 difference = transform_b.get_world().position - transform_a.get_world().position;
                float length = glm::length(difference);
                float force = length - spring.restLength;
                difference /= length;

                glm::vec3 directedForce = difference * force;

                float dt = time->delta_seconds();
                body_a.acceleration += directedForce * body_a.iMass * dt;
                body_b.acceleration -= directedForce * body_b.iMass * dt;

                if (o_transform) {
                    o_transform->position = transform_a.position;
                    o_transform->rotation = glm::quatLookAt(-difference, glm::normalize(o_transform->rotation * glm::vec3(0, 1, 0)));
                    o_transform->scale = {1 / std::sqrt(length), 1 / std::sqrt(length), length};
                }
            }
        }
    };
}
