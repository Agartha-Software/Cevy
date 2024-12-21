/*
** Agartha-Software, 2023
** C++evy
** File description:
** Camera
*/

#pragma once

#include "ecs/Query.hpp"
#include "Target.hpp"
#include "Transform.hpp"
#include "cevy.hpp"
#include "raylib.hpp"

namespace cevy::engine {
class Camera {
  public:
    float fov;
    float aspect;
    float near;
    float far;
    glm::vec3 up;
    float tilt;
    glm::mat4 projection;
    glm::mat4 view;
    Camera3D camera;
  // operator Camera3D &();
  // operator Camera3D *();
  // operator Camera3D() const;
  Camera();
  ~Camera();
};
} // namespace cevy::engine

void update_camera(cevy::ecs::Query<cevy::engine::Camera, option<cevy::engine::Target>,
                                    option<cevy::engine::Transform>>
                       cams);
