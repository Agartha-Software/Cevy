/*
** Agartha-Software, 2023
** C++evy
** File description:
** Camera
*/

#pragma once

#include "Query.hpp"
#include "Target.hpp"
#include "Transform.hpp"
#include "cevy.hpp"
#include "raylib.hpp"

namespace cevy::engine {
class Camera {
  public:
  camera3_d camera;
  operator camera3_d &();
  operator camera3_d *();
  operator camera3_d() const;
  Camera();
  ~Camera();
};
} // namespace cevy::engine

void update_camera(cevy::ecs::Query<cevy::engine::Camera, option<cevy::engine::Target>,
                                    option<cevy::engine::Transform>>
                       cams);
