#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL

#include "Stage.hpp"
#include <cmath>
#include <stdexcept>
#include <vector>

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include "App.hpp"
#include "Assets.hpp"
#include "DeferredRenderer.hpp"
#include "Editor.hpp"
#include "EnginePlugin.hpp"
#include "EntityCommands.hpp"
#include "Mesh.hpp"
#include "Motion.hpp"
#include "PbrMaterial.hpp"
#include "Resource.hpp"
#include "Transform.hpp"
#include "cevy.hpp"
#include "collision/Collider.hpp"
#include "glWindow.hpp"
#include "physics/Physics.hpp"
#include "state.hpp"

using namespace cevy;
using namespace engine;
using namespace ecs;
using namespace physics;

float DEG2RAD = glm::pi<float>() / 180;

struct Ball {};
struct Arrow {};

struct GameState {
  bool is_rolling;
  glm::vec3 arrow_angle;
  float arrow_power = 0.6;
};

void reset(Resource<GameState> game_state, Query<Ball, Transform, RigidBody, Motion> ball,
           Query<Arrow, Transform> arrow, Query<Camera, Transform> camera) {
  auto [_, transform, rigid, motion] = ball.single();

  if (transform.position.z < -2) {
    game_state->is_rolling = false;
    transform = Transform(glm::vec3 {0.1, -8.5, 0.0754}, glm::quat({0, 0, 0}), {1, 1, 1});
    auto [_cam, camera_transform] = camera.single();
    camera_transform = Transform(glm::vec3(0, -12, 1.5),
                                 glm::quat({glm::half_pi<float>() * 0.9, 0, 0}), {1, 1, 1});
    motion.linear = glm::vec3 {0};
    motion.angular = glm::vec3 {0};
    auto [_, arrow_transform] = arrow.single();
    arrow_transform.setScaleY(game_state->arrow_power);
  }
}

void affect_arrow(Resource<input::ButtonInput<input::KeyCode>> keyboard,
                  Query<Arrow, Transform> arrow, Resource<GameState> game_state,
                  Resource<Time> time) {
  if (game_state->is_rolling) {
    return;
  }
  auto [_, arrow_transform] = arrow.single();
  auto rot = arrow_transform.rotation;

  if (keyboard->is_pressed(input::KeyCode::LeftArrow) &&
      game_state->arrow_angle.z < glm::pi<float>() / 5.) {
    game_state->arrow_angle.z += 0.2 * time->delta_seconds();
  }
  if (keyboard->is_pressed(input::KeyCode::RightArrow) &&
      game_state->arrow_angle.z > -glm::pi<float>() / 5.) {
    game_state->arrow_angle.z -= 0.2 * time->delta_seconds();
  }
  if (keyboard->is_pressed(input::KeyCode::Space) &&
      game_state->arrow_angle.x < glm::pi<float>() / 5.) {
    game_state->arrow_angle.x += 0.2 * time->delta_seconds();
  }
  if (keyboard->is_pressed(input::KeyCode::RightShift) && game_state->arrow_angle.x > 0.) {
    game_state->arrow_angle.x -= 0.2 * time->delta_seconds();
  }
  if (keyboard->is_pressed(input::KeyCode::UpArrow) && game_state->arrow_power < 1.2) {
    game_state->arrow_power += 1 * time->delta_seconds();
  }
  if (keyboard->is_pressed(input::KeyCode::DownArrow) && game_state->arrow_power > 0.2) {
    game_state->arrow_power -= 1 * time->delta_seconds();
  }
  arrow_transform.setRotationXYZ(game_state->arrow_angle);
  arrow_transform.setScaleXYZ({0.05, game_state->arrow_power, 0.05});
}

void move_camera(Resource<input::ButtonInput<input::KeyCode>> keyboard,
                 Query<Camera, Transform> cam_q, Resource<ecs::Time> time,
                 Query<Ball, Transform> ball, Query<Arrow, Transform> arrow,
                 Resource<GameState> game_state) {
  if (!game_state->is_rolling) {
    glm::vec3 direction = {0., 0., 0.};
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
  } else {
    for (auto [_camera, transform] : cam_q) {
      auto [_ball, ball_transform] = ball.single();
      transform.setPositionXYZ(ball_transform.position);
      transform.translateY(-4);
      transform.translateZ(2);
      transform.setRotationXYZ({glm::half_pi<float>() * 0.8, 0, 0});
      auto [_, arrow_transform] = arrow.single();
      arrow_transform.setScaleXYZ(0.);
    }
  }
}

void rotate_camera(Query<Camera, Transform> cam_q,
                   Resource<input::ButtonInput<input::MouseButton>> mouse_buttons,
                   EventReader<input::mouseMotion> mouse_motion_reader,
                   Resource<GameState> game_state) {
  static glm::vec2 rotation = {0.f, glm::pi<float>() * 0.3f};

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
                  Resource<Gravity> gravity, EventReader<input::mouseMotion> mouse_motion_reader,
                  Query<Ball, RigidBody, Motion> ball, Resource<GameState> game_state) {
  if (game_state->is_rolling)
    return;
  if (mouse_buttons->is_just_pressed(input::MouseButton::Left)) {
    gravity->acceleration = {0, 0, -9.81};
    auto [_, ball_rigidbody, ball_motion] = ball.single();
    ball_motion.linear +=
        glm::quat(game_state->arrow_angle) * glm::vec3(0., game_state->arrow_power * 15, 0.);
    ball_motion.angular += glm::vec3 {-8., 0., 0.};
    game_state->is_rolling = true;
  }
}

void setup(Resource<asset::AssetManager> asset_manager, Resource<Time> time,
           Resource<Gravity> gravity, Resource<RigidBodyWorld> rigidbody_world,
           Resource<Assets<Mesh>> mesh_manager, Commands cmd) {
  // time->setTimescale(0.01);
  gravity->acceleration = {};
  rigidbody_world->dragDensity = 0.01;
  cmd.spawn(
      SunLight {
          {1., 1., 1.},
      },
      Transform(0., 0., 15.));
  cmd.spawn(
      SunLight {
          {1., 0.9, 0.9},
      },
      Transform(-5., -1., 2.));
  cmd.spawn(Camera(),
            Transform(glm::vec3(0, -12, 1.5), glm::quat({glm::half_pi<float>() * 0.9, 0, 0})));
  std::vector<Handle<PbrMaterial>> pins_mat;
  std::vector<Handle<PbrMaterial>> ball_mat;
  std::vector<Handle<PbrMaterial>> alley_mat;
  auto pins = mesh_manager->add(Mesh::load("./assets/BowlingPins.obj", pins_mat));
  auto ball = mesh_manager->add(Mesh::load("./assets/BowlingBall.obj", ball_mat));
  auto alley = mesh_manager->add(Mesh::load("./assets/BowlingAlley.obj", alley_mat));
  alley->setModelMatrix(glm::mat4(glm::quat(glm::vec3(90. * DEG2RAD, 0, 0))));
  pins->setModelMatrix(
      glm::translate(glm::mat4(glm::quat(glm::vec3(90. * DEG2RAD, 0, 0))), {0, -0.1, 0}));
  RigidBody pin_body(1);
  pin_body.resititution = 0.9;
  pin_body.friction = 0.5;
  pin_body.iInertiaTensor = glm::mat3 {20};
  for (float i = 0; i != 4; i++) {
    float j = 0;
    do {
      cmd.spawn(pins, pins_mat.at(0),
                Transform(glm::vec3(((i / 2) - j) / 3, 8.5 + i / 3, 0.1), glm::quat({0, 0, 0}),
                          {1, 1, 1}),
                Motion()
                // , physics::RigidBody(1), physics::Collider::primitives::Box({0.1, 0.1, 0.3}));
                // , physics::RigidBody(1), physics::Collider(physics::Shape::Sphere({}, 0.001),
                // physics::Shape::Cylinder({0, 0, 0.1}, glm::quat({0, 0, 0}), {0.05, 0.2} ))); ,
                // physics::RigidBody(1), physics::Collider::primitives::Sphere(0.1));
                ,
                pin_body,
                physics::Collider(
                    physics::Shape::Cylinder({0, 0, 0.05}, glm::quat({0, 0, 0}), {0.05, 0.3}),
                    physics::Shape::Box({0, 0, 0.05}, {0.03, 0.03, 0.25})));
      j++;
    } while (j <= i);
  }
  cmd.spawn(Ball {}, ball, ball_mat.at(0),
            Transform(glm::vec3(0.1, -8.5, 0.0754), glm::quat({0, 0, 0})), RigidBody(8.),
            Motion({0, 0, 0}), Collider::primitives::Sphere(.0753));
  cmd.spawn(alley, alley_mat.at(0), Transform(), RigidBody::Passive(), Motion(),
            Collider::primitives::Quad({0.7, 9.6}, glm::quat({0, 0, 0}, {0, 0, 0})));

  auto mat_white = asset_manager->add(PbrMaterial(), "white.material");
  auto cube_handle = asset_manager->add(primitives::cube(1), "cube.mesh");
  cube_handle->setModelMatrix(Transform(glm::vec3 {0, 0.6, 0}));

  cmd.spawn(Arrow {}, cube_handle, mat_white,
            Transform(glm::vec3(0.1, -8.5, 0.0754), glm::quat({0., 0., 0.}), {0.05, 0.6, 0.05}));
}

int main() {
  App app;
  app.add_plugins(engine::Engine<glWindow::Builder<engine::DeferredRenderer>>());
  // app.add_plugins(Engine<glWindow>());
  app.add_plugins(PhysicsPlugin());
  app.init_component<Ball>();
  app.init_component<Arrow>();
  GameState state = GameState {false};
  app.init_resource(state);
  app.add_systems<core_stage::Startup>(setup);
  app.add_systems<core_stage::Update>(rotate_camera);
  app.add_systems<core_stage::Update>(move_camera);
  app.add_systems<core_stage::Update>(ball_control);
  app.add_systems<core_stage::Update>(reset);
  app.add_systems<core_stage::Update>(affect_arrow);
  app.run();
  return 0;
}
