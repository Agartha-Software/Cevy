/*
** Agartha-Software, 2023
** C++evy
** File description:
** Camera
*/

#include "Camera.hpp"
#include "Target.hpp"
#include "Transform.hpp"
#include "cevy.hpp"
#include "raymath.h"

cevy::engine::Camera::Camera() {
  this->camera = {
      (Vector3){10.0f, 10.0f, 10.0f},
      (Vector3){0.0f, 0.0f, 0.0f},
      (Vector3){0.0f, 0.0f, 90.0f},
      45.0f,
      CAMERA_PERSPECTIVE,
  };
}

cevy::engine::Camera::~Camera() {}

cevy::engine::Camera::operator camera3_d &() { return this->camera; }

cevy::engine::Camera::operator camera3_d *() { return &this->camera; }

cevy::engine::Camera::operator camera3_d() const { return this->camera; }

void update_camera(cevy::ecs::Query<cevy::engine::Camera, option<cevy::engine::Target>,
                                    option<cevy::engine::Transform>>
                       cams) {
  for (auto [cam, opt_target, opt_transform] : cams) {
    if (opt_transform) {
      auto &tm = opt_transform.value();
      cam.camera.up = Vector3RotateByQuaternion(Vector3{0, 1, 0}, tm.rotation);
      if (opt_target) {
        cam.camera.target = opt_target.value();
      } else {
        Vector3 vc = Vector3RotateByQuaternion(Vector3{0, 0, 1}, tm.rotation);
        cam.camera.target = Vector3Add(tm.position, vc);
      }
      cam.camera.position = tm.position;
    }
  }
}
