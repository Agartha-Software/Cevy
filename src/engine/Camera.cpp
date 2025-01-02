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

cevy::engine::Camera::Camera() {
  this->fov = 70;
  this->aspect = 1;
  this->near = 0.01;
  this->far = 500;
  this->projection = glm::perspective(fov, aspect, near, far);
  this->view = glm::mat4(1);
}

cevy::engine::Camera::~Camera() {}

void update_camera(cevy::ecs::Query<cevy::engine::Camera, option<cevy::engine::Target>,
                                    option<cevy::engine::Transform>>
                       cams) {
  for (auto [cam, opt_target, opt_transform] : cams) {
    if (opt_transform) {
      auto &tm = opt_transform.value();
      cam.view = tm;
    }
  }
}
