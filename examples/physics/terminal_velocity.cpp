#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <stdexcept>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include "cevy.hpp"
#include "App.hpp"
#include "Assets.hpp"
#include "Color.hpp"
#include "DeferredRenderer.hpp"
#include "EnginePlugin.hpp"
#include "EntityCommands.hpp"
#include "Mesh.hpp"
#include "PbrMaterial.hpp"
#include "Transform.hpp"
#include "Velocity.hpp"
#include "collision/Collider.hpp"
#include "glWindow.hpp"
#include "physics/Physics.hpp"
#include "state.hpp"

using namespace cevy;
using namespace ecs;
using namespace engine;

struct Origin {
  Transform transform;
};

struct ResetClock {
  double time;
};

struct Raycaster {
  physics::Collider hitPlane = physics::Collider(physics::Shape::Plane({},{0, 0, 1}));
};

float DEG2RAD = glm::pi<float>() / 180;

static glm::vec3 hsv2rgb(glm::vec3 c) {
  glm::vec4 K = glm::vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  glm::vec3 p = abs(fract(c.xxx() + K.xyz()) * 6.0f - K.www());
  return c.z * mix(K.xxx(), clamp(p - K.xxx(), 0.0f, 1.0f), c.y);
}

int initial_setup(Resource<asset::AssetManager> asset_manager,
                  Resource<Assets<Mesh>> mesh_manager,
                  Resource<Assets<PbrMaterial>> material_manager,
                  Resource<physics::Gravity> gravity,
                  Resource<Time> time,
                  Commands cmd) {
  // gravity->acceleration = {};
  time->reset();
  // time->setTimescale(0.75);

  auto plane_handle = asset_manager->add(primitives::plane(16, 8, 8), "plane.mesh");

  // auto plane_handle = mesh_manager->add();
  auto sphere = primitives::sphere(1, 32, 16);
  // sphere.setModelMatrix(glm::mat4(Transform(0, 0, 1.5)));

  auto sphere_handle = asset_manager->add(std::move(sphere), "sphere.mesh");

  auto mat_white = asset_manager->add(PbrMaterial(), "white.material");
  auto mat_sphere = asset_manager->add(PbrMaterial(glm::vec3(0.8, 0.8, 0.8), glm::vec3(1), 12), "sphere.material");
  auto _camera =
      cmd.spawn(Camera(), Transform(glm::vec3(0, -10, 5),
                                    glm::quat({glm::half_pi<float>() * 0.8, 0, 0}), glm::vec3(1)));

  auto origin_a = Origin {
      Transform({0, 0, 16}, glm::quat({0, 0, 0,}), glm::vec3(1)),
  };

  auto sphere_a =
      cmd.spawn(sphere_handle, mat_sphere, Color(1, 0.1, 0.1), origin_a, origin_a.transform,
                TransformVelocity(), physics::RigidBody(1000), physics::Collider(physics::Shape::Sphere({}, 1)));
  auto sphere_b =
  cmd.spawn(plane_handle, mat_white, Color(0.8, 0.8, 1), Transform({0, 0, 0}, glm::quat({0, 0, 0}), {1, 1, 1}));
  cmd.spawn(plane_handle, mat_white, Color(0.8, 0.8, 1), Transform({0, 0, 16}, glm::quat({0, glm::radians(180.f), 0}), {1, 1, 1}));
  // cmd.spawn(PointLight{{100, 100 ,100}, 1, 30}, Transform(0, 0, 10));
  // cmd.spawn(SpotLight{{100, 100 ,100}, 0.2, 0.8}, Transform(0, 0, 15));


  glm::vec3 sun_pos = {16, -20, 16};
  glm::quat sun_rot = glm::quatLookAt(-glm::normalize(sun_pos), glm::vec3(0, 0, 1));
//   cmd.spawn(SunLight{{1, 1 ,1}, 40, 40}, Transform(sun_pos, sun_rot, {1, 1, 1}));
  cmd.spawn(SunLight{{1, 1 ,1}, 40, 40}, Transform(sun_pos, sun_rot, {1, 1, 1}));
  return 0;
}

void pos_reset(Resource<Time> time,
               Query<Entity, const Origin, Transform> origins) {
for (auto [entity, origin, tm] : origins) {
    if (tm.position.z < 0) {
        tm.position = origin.transform.position;
    }
}
}

void log_speed(Resource<Time> time, Query<Entity, TransformVelocity> entities) {
    for (auto [en, vel]: entities) {
        std::cout << "@" << time->uptime().count() << ": " << cevy::reflect(vel.position) << std::endl;
    }
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
}

int main() {
  App app;
  // app.add_plugins(Engine<glWindow::Builder<cevy::engine::DeferredRenderer, editor::Editor>>());
  app.add_plugins(Engine<glWindow::Builder<cevy::engine::DeferredRenderer>>());
  // app.add_plugins(Engine<glWindow>());
  app.add_plugins(physics::PhysicsPlugin());
  app.init_component<Origin>();
  app.init_resource<ResetClock>();
  app.init_resource<Raycaster>();
  app.add_systems<core_stage::Startup>(initial_setup);
  app.add_systems<core_stage::Update>(rotate_camera);
  app.add_systems<core_stage::Update>(move_camera);
  app.add_systems<core_stage::Update>(pos_reset);
  app.add_systems<core_stage::Update>(log_speed);
  app.run();
}
