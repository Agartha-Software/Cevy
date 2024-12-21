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
#include <glm/ext/matrix_clip_space.hpp>
#include "raymath.h"

cevy::engine::Camera::Camera() {
  this->projection = glm::perspective(fov, aspect, near, far);
  this->view = glm::mat4(1);
  this->camera = {
      (Vector3){10.0f, 10.0f, 10.0f},
      (Vector3){0.0f, 0.0f, 0.0f},
      (Vector3){0.0f, 0.0f, 90.0f},
      45.0f,
      CAMERA_PERSPECTIVE,
  };
}

cevy::engine::Camera::~Camera() {}

// cevy::engine::Camera::operator Camera3D &() { return this->camera; }

// cevy::engine::Camera::operator Camera3D *() { return &this->camera; }

// cevy::engine::Camera::operator Camera3D() const { return this->camera; }

void update_camera(cevy::ecs::Query<cevy::engine::Camera, option<cevy::engine::Target>,
                                    option<cevy::engine::Transform>>
                       cams) {
  for (auto [cam, opt_target, opt_transform] : cams) {
    if (opt_transform) {
      auto &tm = opt_transform.value();
      cam.view = tm;
      Quaternion quat({tm.rotation.x, tm.rotation.y, tm.rotation.z, tm.rotation.w});
      cam.camera.up = Vector3RotateByQuaternion(Vector3{0, 1, 0}, quat);
      if (opt_target) {
        glm::vec3 target = opt_target.value();
        cam.camera.target = Vector3{target.x, target.y, target.z};
      } else {
        Vector3 vc = Vector3RotateByQuaternion(Vector3{0, 0, 1}, quat);
        cam.camera.target = Vector3Add(Vector3{tm.position.x, tm.position.y, tm.position.z}, vc);
      }
      cam.camera.position = Vector3{tm.position.x, tm.position.y, tm.position.z};
    }
  }
}
