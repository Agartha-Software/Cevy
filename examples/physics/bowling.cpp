#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL

#include "cevy.hpp"
#include <stdexcept>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>

#include "App.hpp"
#include "Assets.hpp"
#include "Color.hpp"
#include "DeferredRenderer.hpp"
#include "EnginePlugin.hpp"
#include "EntityCommands.hpp"
#include "Mesh.hpp"
#include "PbrMaterial.hpp"
#include "Transform.hpp"
#include "Motion.hpp"
#include "collision/Collider.hpp"
#include "glWindow.hpp"
#include "physics/Physics.hpp"
#include "state.hpp"
#include "Editor.hpp"

using namespace cevy;
using namespace engine;
using namespace ecs;

float DEG2RAD = glm::pi<float>() / 180;


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
    if (keyboard->is_pressed(input::KeyCode::Q)) {
      direction.y -= 1;
    }
    if (keyboard->is_pressed(input::KeyCode::E)) {
      direction.y += 1;
    }
    if (keyboard->is_pressed(input::KeyCode::W)) {
      direction.z -= 1;
    }
    if (keyboard->is_pressed(input::KeyCode::S)) {
      direction.z += 1;
    }
    float delta_time = time->raw().count();

    if (glm::length(direction) != 0) {
      transform.translateXYZ(transform.rotation * glm::normalize(direction) * speed * delta_time);
    }
  }
}

void rotate_camera(Query<Camera, Transform> cam_q,
                  Resource<input::ButtonInput<input::MouseButton>> mouse_buttons,
                  cevy::ecs::EventReader<input::mouseMotion> mouse_motion_reader) {
  static glm::vec2 rotation = {0 * glm::pi<float>(), glm::pi<float>() * 0.3f};

  if (mouse_buttons->is_pressed(input::MouseButton::Right)) {
    // std::cout << "mousing!" << std::endl;
    for (const auto &mouse_motion : mouse_motion_reader) {
      if (mouse_motion.delta.has_value()) {
        rotation.x -= mouse_motion.delta.value().x * 0.005;
        rotation.y -= mouse_motion.delta.value().y * 0.005;
        rotation.y = glm::clamp(rotation.y, 0.f, glm::pi<float>());
        // std::cout << "motion!" << mouse_motion.delta.value().x << " " << (mouse_motion.delta.value().y) << std::endl;
      } else {
        throw std::runtime_error("mouse motion has no value!!");
      }
      auto xQuat = glm::quat({0., 0., rotation.x});
      auto yQuat = glm::quat({rotation.y, 0., 0.});
      for (auto [_, transform] : cam_q) {
        transform.rotation = xQuat * yQuat;
      }
    }
  }
}

void setup(Resource<asset::AssetManager> asset_manager,
  Resource<Assets<Mesh>> mesh_manager,
  cevy::ecs::Commands cmd) {
  cmd.spawn(SunLight{{1, 1 , 1}, }, Transform(0, 0, 15));
  cmd.spawn(SunLight{{1, 0.9, 0.9}, }, Transform(-5, -1, 2));
  cmd.spawn(Camera(), Transform(glm::vec3(0, -10, 5),
                                    glm::quat({glm::half_pi<float>() * 0.8, 0, 0}), glm::vec3(1)));
  std::vector<Handle<PbrMaterial>> pins_mat;
  std::vector<Handle<PbrMaterial>> ball_mat;
  std::vector<Handle<PbrMaterial>> alley_mat;
  auto pins = mesh_manager->add(Mesh::load("./assets/BowlingPins.obj", pins_mat));
  auto ball = mesh_manager->add(Mesh::load("./assets/BowlingBall.obj", ball_mat));
  auto alley = mesh_manager->add(Mesh::load("./assets/BowlingAlley.obj", alley_mat));
  auto rotation = glm::quat(glm::vec3(90. * glm::pi<float>() / 180, 0, 0));
  for (float i = 0; i != 4; i++) {
    float j = 0;
    do {
      cmd.spawn(pins, pins_mat.at(0), Transform(glm::vec3(((i / 2) - j) / 3, 9 + i / 3, 0), rotation, glm::vec3(1)));
      j++;
    } while (j <= i);
  }
  cmd.spawn(ball, ball_mat.at(0), Transform(glm::vec3(0, -1, 0.07515), glm::quat(), glm::vec3(1)));
  cmd.spawn(alley, alley_mat.at(0), Transform(glm::vec3(0, 0, 0), rotation, glm::vec3(1.1)), physics::RigidBody(INFINITY), physics::Collider::primitives::Quad({8, 8}));
}

int main() {
  App app;
  app.add_plugins(engine::Engine<glWindow::Builder<engine::DeferredRenderer>>());
  // app.add_plugins(Engine<glWindow>());
  app.add_plugins(physics::PhysicsPlugin());
  app.add_systems<core_stage::Startup>(setup);
    app.add_systems<core_stage::Update>(rotate_camera);
  app.add_systems<core_stage::Update>(move_camera);
  app.run();
    return 0;
}
