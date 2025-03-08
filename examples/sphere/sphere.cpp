#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include "App.hpp"
#include "Asset.hpp"
#include "AssetManager.hpp"
#include "Color.hpp"
#include "DeferredRenderer.hpp"
#include "Engine.hpp"
#include "Model.hpp"
#include "PbrMaterial.hpp"
#include "Transform.hpp"
#include "Velocity.hpp"
#include "commands/EntityCommands.hpp"
#include "glWindow.hpp"

using namespace cevy;
using namespace ecs;
using namespace engine;

float DEG2RAD = glm::pi<float>() / 180;

static glm::vec3 hsv2rgb(glm::vec3 c) {
  glm::vec4 K = glm::vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  glm::vec3 p = abs(fract(c.xxx() + K.xyz()) * 6.0f - K.www());
  return c.z * mix(K.xxx(), clamp(p - K.xxx(), 0.0f, 1.0f), c.y);
}

int initial_setup(Resource<Asset<Model>> mesh_manager,
                  Resource<Asset<PbrMaterial>> material_manager, Commands cmd) {
  auto plane_handle = mesh_manager->load(primitives::plane(7, 4, 4));
  auto sphere = primitives::sphere(1, 32, 16);
  sphere.setModelMatrix(glm::mat4(Transform(0, 0, 0.5)));

  auto sphere_handle = mesh_manager->load(std::move(sphere));
  auto mat_white = material_manager->load(PbrMaterial());
  auto mat_sphere = material_manager->load(PbrMaterial(glm::vec3(0.1, .1, .1), glm::vec3(1), 12));
  cmd.spawn(Camera(), Transform(glm::vec3(0, -10, 5),
                                glm::quat({glm::half_pi<float>() * 0.8, 0, 0}), glm::vec3(1)));

  auto rotator = cmd.spawn(sphere_handle, mat_sphere, Color(0, 0, 1), Transform(),
                           TransformVelocity(glm::quat({0, 0, DEG2RAD * 90})));

  cmd.spawn(plane_handle, mat_white, Color(0.8, 0.8, 1), Transform());

  const int ringCount = 9;
  const float ringRadius = 5;
  for (int i = 0; i < ringCount; i++) {
    glm::vec3 rgb = 10.f * hsv2rgb({float(i) / ringCount, 0.9, 1.0f});
    auto mat_light = material_manager->load(PbrMaterial(glm::vec3(), glm::vec3(), 1));
    mat_light->emit = rgb;
    Transform tm = Transform(
        glm::vec3(ringRadius * std::cos(glm::two_pi<float>() * float(i) / ringCount),
                  ringRadius * std::sin(glm::two_pi<float>() * float(i) / ringCount), 3.0f),
        glm::quat(), glm::vec3(.5, .5, .5));
    PointLight light = {rgb, 1.0f};
    auto entity = cmd.spawn(Parent {rotator.id()}, tm, light, sphere_handle, mat_light);
  }

  return 0;
}

void move_camera(Resource<input::ButtonInput<input::KeyCode>> keyboard,
                 Query<Camera, Transform> cam_q, Resource<ecs::Time> time) {
  glm::vec3 direction = {0, 0, 0};
  float speed = 10;

  for (auto [_, transform] : cam_q) {
    if (keyboard->is_pressed(input::KeyCode::A)) {
      direction.x -= 1;
    }
    if (keyboard->is_pressed(input::KeyCode::D)) {
      direction.x += 1;
    }
    if (keyboard->is_pressed(input::KeyCode::Shift)) {
      direction.y -= 1;
    }
    if (keyboard->is_pressed(input::KeyCode::Space)) {
      direction.y += 1;
    }
    if (keyboard->is_pressed(input::KeyCode::W)) {
      direction.z -= 1;
    }
    if (keyboard->is_pressed(input::KeyCode::S)) {
      direction.z += 1;
    }
    float delta_time = time->delta().count();

    if (glm::length(direction) != 0) {
      transform.translateXYZ(transform.rotation * glm::normalize(direction) * speed * delta_time);
    }
  }
}

void rotate_camera(Query<Camera, Transform> cam_q,
                   cevy::ecs::EventReader<input::mouseMotion> mouse_motion_reader) {
  static glm::vec2 rotation = {0 * glm::pi<float>(), glm::pi<float>() * 0.3f};

  for (const auto &mouse_motion : mouse_motion_reader) {
    if (mouse_motion.delta.has_value()) {
      rotation.x -= mouse_motion.delta.value().x * 0.005;
      rotation.y -= mouse_motion.delta.value().y * 0.005;
      rotation.y = glm::clamp(rotation.y, 0.f, glm::pi<float>());
    }
    auto xQuat = glm::quat({0., 0., rotation.x});
    auto yQuat = glm::quat({rotation.y, 0., 0.});
    for (auto [_, transform] : cam_q) {
      transform.rotation = xQuat * yQuat;
    }
  }
}

int main() {
  App app;
  app.init_resource<AssetManager>();
  app.add_plugins(Engine<glWindow, DeferredRenderer>());
  app.add_systems<core_stage::PostStartup>(initial_setup);
  app.add_systems<core_stage::Update>(rotate_camera);
  app.add_systems<core_stage::Update>(move_camera);
  app.run();
}
