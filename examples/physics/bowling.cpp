#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL

#include <stdexcept>
#include <cmath>
#include <vector>

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/ext/vector_float3.hpp>

#include "cevy.hpp"
#include "App.hpp"
#include "Assets.hpp"
#include "DeferredRenderer.hpp"
#include "EnginePlugin.hpp"
#include "Mesh.hpp"
#include "PbrMaterial.hpp"
#include "Transform.hpp"
#include "Motion.hpp"
#include "collision/Collider.hpp"
#include "glWindow.hpp"
#include "physics/Physics.hpp"
#include "Resource.hpp"
#include "state.hpp"
#include "EntityCommands.hpp"
#include "Editor.hpp"

using namespace cevy;
using namespace engine;
using namespace ecs;

float DEG2RAD = glm::pi<float>() / 180;

struct Ball {};

struct GameState {
  bool is_rolling;
};

void reset(Resource<GameState> game_state, Query<Ball, Transform, cevy::physics::RigidBody, cevy::engine::Motion> ball, Query<Camera, Transform> camera) {
  auto [_, transform, rigid, motion] = ball.single();
  if (transform.position.z < -2) {
    game_state->is_rolling = false;
    transform = Transform(glm::vec3(0, -8.5, 0.0754), glm::quat({0, 0, 0}));
    auto [_cam, camera_transform] = camera.single();
    camera_transform = Transform(glm::vec3(0, -10, 5), glm::quat({glm::half_pi<float>() * 0.8, 0, 0}));
    motion.linear = glm::vec3{0};
    motion.angular = glm::vec3{0};
  }
}

void move_camera(Resource<input::ButtonInput<input::KeyCode>> keyboard,
                 Query<Camera, Transform> cam_q, Resource<ecs::Time> time,
                 Query<Ball, Transform> ball,
                 Resource<GameState> game_state) {
  if (game_state->is_rolling == false) {
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
      std::cout << "tra" << transform.position.x << " " << transform.position.y << " " << transform.position.z << " " << std::endl;
      std::cout << "rot" << transform.rotation.x << " " << transform.rotation.y << " " << transform.rotation.z << " " << std::endl;
    }
  } else {
    for (auto [_camera, transform] : cam_q) {
      auto [_ball, ball_transform] = ball.single();
      transform.setPositionXYZ(ball_transform.position);
      transform.translateY(-4);
      transform.translateZ(2);
      transform.setRotationXYZ({glm::half_pi<float>() * 0.8, 0, 0});
    }
  }
}

void rotate_camera(Query<Camera, Transform> cam_q,
                  Resource<input::ButtonInput<input::MouseButton>> mouse_buttons,
                  cevy::ecs::EventReader<input::mouseMotion> mouse_motion_reader,
                  Resource<GameState> game_state) {
  static glm::vec2 rotation = {0 * glm::pi<float>(), glm::pi<float>() * 0.3f};

  if (mouse_buttons->is_pressed(input::MouseButton::Right)) {
    for (const auto &mouse_motion : mouse_motion_reader) {
      if (mouse_motion.delta.has_value()) {
        rotation.x -= mouse_motion.delta.value().x * 0.005;
        rotation.y -= mouse_motion.delta.value().y * 0.005;
        rotation.y = glm::clamp(rotation.y, 0.f, glm::pi<float>());
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

void ball_control(Resource<input::ButtonInput<input::MouseButton>> mouse_buttons,
                  cevy::ecs::EventReader<input::mouseMotion> mouse_motion_reader,
                  Query<Ball, cevy::physics::RigidBody, cevy::engine::Motion> ball,
                  Resource<GameState> game_state
                ) {
  if (game_state->is_rolling == true) {
    return;
  }
  if (mouse_buttons->is_just_pressed(input::MouseButton::Left)) {
    auto [_, ball_rigidbody, ball_motion] = ball.single();
    ball_motion.linear += glm::vec3{0., 10., 0.};
    ball_motion.angular += glm::vec3{-20., 0., 0.};
    game_state->is_rolling = true;
  }
}

void setup(Resource<asset::AssetManager> asset_manager,
  Resource<Time> time,
  Resource<physics::Gravity> gravity,
  Resource<physics::RigidBodyWorld> rigidbody_world,
  Resource<Assets<Mesh>> mesh_manager,
  cevy::ecs::Commands cmd) {
  rigidbody_world->dragDensity = 0.01;
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
  alley->setModelMatrix(glm::mat4(glm::quat(glm::vec3(90. * DEG2RAD, 0, 0))));
  pins->setModelMatrix(glm::mat4(glm::quat(glm::vec3(90. * DEG2RAD, 0, 0))));
  for (float i = 0; i != 4; i++) {
    float j = 0;
    do {
      cmd.spawn(pins, pins_mat.at(0), Transform(glm::vec3(((i / 2) - j) / 3, 8.5 + i / 3, 0), glm::quat({0, 0, 0}), glm::vec3(1)));
      j++;
    } while (j <= i);
  }
  cmd.spawn(Ball {}, ball, ball_mat.at(0), Transform(glm::vec3(0, -8.5, 0.0754), glm::quat({0, 0, 0}), glm::vec3(1)), physics::RigidBody(1.), Motion({0, 0, 0}), physics::Collider::primitives::Sphere(.0753));
  cmd.spawn(alley, alley_mat.at(0), Transform(glm::vec3(0, 0, 0), glm::quat({0, 0, 0}), glm::vec3(1)), physics::RigidBody(INFINITY), Motion(), physics::Collider::primitives::Quad({10.5, 10.5}));
}

int main() {
  App app;
  app.add_plugins(engine::Engine<glWindow::Builder<engine::DeferredRenderer>>());
  // app.add_plugins(Engine<glWindow>());
  app.add_plugins(physics::PhysicsPlugin());
  app.init_component<Ball>();
  GameState state = GameState { false };
  app.init_resource(state);
  app.add_systems<core_stage::Startup>(setup);
  app.add_systems<core_stage::Update>(rotate_camera);
  app.add_systems<core_stage::Update>(move_camera);
  app.add_systems<core_stage::Update>(ball_control);
  app.add_systems<core_stage::Update>(reset);
  app.run();
  return 0;
}
