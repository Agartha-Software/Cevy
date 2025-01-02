/*
** Agartha-Software, 2023
** C++evy
** File description:
** Camera
*/

#pragma once

#include "Target.hpp"
#include "Transform.hpp"
#include "cevy.hpp"
#include "ecs/Query.hpp"

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
  Camera();
  ~Camera();
};
} // namespace cevy::engine

void update_camera(cevy::ecs::Query<cevy::engine::Camera, option<cevy::engine::Target>,
                                    option<cevy::engine::Transform>>
                       cams);
