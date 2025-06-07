#include "RigidBody.hpp"
#include <cmath>
#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/ext/quaternion_transform.hpp>
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
#include "Velocity.hpp"
#include "collision/Collider.hpp"
#include "glWindow.hpp"
#include "physics/Physics.hpp"
#include "state.hpp"

using namespace cevy;
using namespace ecs;
using namespace engine;

struct Raycaster {
  physics::Collider hitPlane = physics::Collider::primitives::Quad({100, 100});
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
                  Resource<physics::RigidBodyWorld> world,
                  Commands cmd) {
  gravity->acceleration = {};
  time->reset();
  world->dragDensity *= 0.1;
  // time->setTimescale(0.75);

  auto plane_handle = asset_manager->add(primitives::plane(16, 8, 8), "plane.mesh");
  auto cube_handle = asset_manager->add(primitives::cube(1), "cube.mesh");
  // cube_handle->setModelMatrix(glm::scale(glm::translate(glm::mat4(1), {0, 0, 0.5}), {0.05, 0.05, 0.5}));

  // auto plane_handle = mesh_manager->add();
  auto sphere = primitives::sphere(1, 32, 16);
  // sphere.setModelMatrix(glm::mat4(Transform({0, 0, 0}, glm::quat({0, 0, 0}), {0.1, 0.1, 0.1})));

  auto sphere_handle = asset_manager->add(std::move(sphere), "sphere.mesh");

  auto mat_white = asset_manager->add(PbrMaterial(), "white.material");
  // auto mat_sphere = asset_manager->add(PbrMaterial(glm::vec3(0.8, 0.8, 0.8), glm::vec3(1), 12), "sphere.material");
  auto _camera =
      cmd.spawn(Camera(), Transform(glm::vec3(0, -10, 5),
                                    glm::quat({glm::half_pi<float>() * 0.8, 0, 0}), glm::vec3(1)));

  cmd.spawn(plane_handle, mat_white, Color(0.8, 0.8, 1), Transform(), physics::RigidBody(INFINITY), physics::Collider::primitives::Quad({100, 100}));
  cmd.spawn(SunLight{{1, 1 ,1},}, Transform(0, 0, 15));


  cmd.spawn(cube_handle, mat_white, Transform(glm::vec3(0, 0, 2), glm::quat({0, 0, 0}), {0.1, 0.1, 0.1}), Motion(), physics::RigidBody(0.5), physics::Collider(physics::Shape::Sphere({0, 0, 0}, 1)));


  return 0;
}

void click_sys(
  Query<Transform, option<Motion>> transforms, Commands cmd, Query<const Camera, const Transform> camera,
  Query<Entity, Transform, physics::Collider, option<physics::RigidBody>> colliders,
  Resource<input::ButtonInput<input::MouseButton>> mouse_buttons,
  Resource<input::ButtonInput<input::KeyCode>> keyboard,
  Resource<asset::AssetManager> asset_manager,
  cevy::ecs::Resource<cevy::input::cursorPosition> cursorPosition,
  cevy::ecs::Resource<Raycaster> raycaster,
  cevy::ecs::Resource<Window> window,
  Resource<Time> time
) {
  auto o_camera = camera.get_single();
  static std::optional<Entity> o_ent = {};
  static float ent_mass;
  static glm::vec3 grab_pos;

  static std::optional<Entity> marker_a = {};
  static std::optional<Entity> marker_b = {};
  static std::optional<Entity> marker_c = {};


  if (!marker_a) {
    marker_a = cmd.spawn(asset_manager->get<Mesh>("sphere.mesh").value(), asset_manager->get<PbrMaterial>("white.material").value(), Transform({}, glm::quat({0, 0, 0}), {0, 0, 0}), Color(1, 0, 0)).id();
    return;
  }
  if (!marker_b) {
    marker_b = cmd.spawn(asset_manager->get<Mesh>("sphere.mesh").value(), asset_manager->get<PbrMaterial>("white.material").value(), Transform({}, glm::quat({0, 0, 0}), {0, 0, 0}), Color(0, 0, 1)).id();
    return;
  }
  if (!marker_c) {
    marker_c = cmd.spawn(asset_manager->get<Mesh>("sphere.mesh").value(), asset_manager->get<PbrMaterial>("white.material").value(), Transform({}, glm::quat({0, 0, 0}), {0, 0, 0}), Color(0, 1, 0)).id();
    return;
  }

  if (!o_ent && mouse_buttons->is_pressed(input::MouseButton::Left) && o_camera) {
    std::cout << "click" << std::endl;
    auto [camera, cam_trans] = o_camera.value();
    auto window_size = window->windowSize();
    glm::vec2 screen_space = { (float(cursorPosition->pos.x) / window_size.x) * 2.f - 1.f, ((float(cursorPosition->pos.y) / window_size.y) * -2.f + 1.f) / window_size.x * window_size.y};
    // screen_space = glm::inverse(camera.projection) * glm::vec4(screen_space, 1, 0);
    glm::mat4 cam_tm = glm::mat4(cam_trans);
    cam_tm /= cam_tm[3][3];
    auto origin = cam_tm * glm::vec4(0, 0, 0, 1);
    auto direction = glm::inverse(camera.projection) * glm::vec4(screen_space, 1, 1);
    direction = cam_tm * glm::vec4(direction.xyz(), 0);
    origin /= origin.w;
    auto ray = physics::Ray{origin, glm::normalize(direction.xyz())};

    std::vector<std::pair<physics::Collision, Entity>> collisions;
    for (auto [en, tm, collider, body] : colliders) {
      if ((o_ent && o_ent.value() == en) || (body && body->iMass == 0)) {
        std::cout << "skipped:" << o_ent.has_value() << " " << (body ? body->iMass : NAN) << std::endl;
        continue;
      }
      collisions.push_back({collider.raycast(tm, ray), en});
    }

    std::cout << collisions.size() << " collisions processed" << std::endl;

    auto collision = physics::Ray::filter(ray, collisions.begin(), collisions.end());

    if (collision == collisions.end() || !collision->first.hit) {
      std::cout << "no hit" << std::endl;
      return;
    }
    o_ent.emplace(collision->second);
    auto clicked = colliders.get(collision->second).value();
    auto &o_body = std::get<option<physics::RigidBody>&>(clicked);
    const auto &tm = std::get<Transform&>(clicked);
    // if (o_body) {
    //   ent_mass = o_body->iMass;
    //   o_body->iMass = 0;
    // }
    grab_pos = collision->first.location - tm.position;
    grab_pos = glm::inverse(tm.rotation) * grab_pos;

    // auto [marker_a_tm, _a] = transforms.get(marker_a.value()).value();
    // marker_a_tm.position = + tm.position + tm.rotation * (grab_pos * 1.5f);
    // marker_a_tm.scale = {0.2, 0.2, 0.2};

    // auto [marker_b_tm, _b] = transforms.get(marker_b.value()).value();
    // marker_b_tm.position = collision->first.location;
    // marker_b_tm.scale = {0.2, 0.2, 0.2};
  }
  if (mouse_buttons->is_released(input::MouseButton::Left) && o_ent) {
    std::cout << "releasing object" << std::endl;
    auto o_body = std::get<option<physics::RigidBody>&>(colliders.get(o_ent.value()).value());
    if (o_body) {
      o_body->iMass = ent_mass;
      std::cout << "mass reset to" << ent_mass << std::endl;
    }
    auto [_, motion] = transforms.get(o_ent.value()).value();
    if (motion) {
      motion->animated = false;
      std::cout << "animation reset to" << motion->animated << std::endl;
    }
    auto [marker_a_tm, _m_a_v] = transforms.get(marker_a.value()).value();
    marker_a_tm.scale = {0, 0, 0};
    auto [marker_b_tm, _m_b_v] = transforms.get(marker_b.value()).value();
    marker_b_tm.scale = {0, 0, 0};
    auto [marker_c_tm, _m_c_v] = transforms.get(marker_c.value()).value();
    marker_c_tm.scale = {0, 0, 0};
    o_ent.reset();
    std::cout << "released object" << std::endl;
  }
  if (o_ent) {
    auto [camera, cam_trans] = o_camera.value();
    auto window_size = window->windowSize();
    glm::vec2 screen_space = { (float(cursorPosition->pos.x) / window_size.x) * 2.f - 1.f, ((float(cursorPosition->pos.y) / window_size.y) * -2.f + 1.f) / window_size.x * window_size.y};
    // screen_space = glm::inverse(camera.projection) * glm::vec4(screen_space, 1, 0);
    glm::mat4 cam_tm = glm::mat4(cam_trans);
    cam_tm /= cam_tm[3][3];
    auto origin = cam_tm * glm::vec4(0, 0, 0, 1);
    auto direction = glm::inverse(camera.projection) * glm::vec4(screen_space, 1, 1);
    direction = cam_tm * glm::vec4(direction.xyz(), 0);
    origin /= origin.w;
    auto ray = physics::Ray{origin, glm::normalize(direction.xyz())};

    auto [transform, motion] = transforms.get(o_ent.value()).value();


    glm::mat4 plane_tm = glm::translate(glm::mat4(1), transform.position);
    if (keyboard->is_pressed(input::KeyCode::Space)) {
      plane_tm = plane_tm * glm::mat4(cam_trans.rotation);
    }

    auto collision = raycaster->hitPlane.raycast(plane_tm, ray);
    if (!collision.hit)
      return;
    auto tug_point = transform.rotation * grab_pos;

    // std::cout << reflect(tug_point) << std::endl;
    auto tug = collision.location - tug_point - transform.position;
    // tug *= tug_force;

    auto point_velocity = glm::cross(tug_point, motion->angular.xyz() * motion->angular.w) + motion->linear;

    float damping = glm::min(1.0, 20.0 * time->delta_seconds());

    tug += point_velocity * damping;

    auto tug_out_component = glm::normalize(tug_point) * glm::dot(tug, tug_point);
    auto angular_tug = tug - tug_out_component;


    auto axis = glm::normalize(glm::cross(tug_point, tug));


    auto angle = glm::length(angular_tug) * glm::length(tug_point);
    auto [marker_a_tm, _a] = transforms.get(marker_a.value()).value();
    auto [marker_b_tm, _b] = transforms.get(marker_b.value()).value();
    auto [marker_c_tm, _c] = transforms.get(marker_c.value()).value();
    tug_point *= 1.5f;
    // {
    //   marker_a_tm.position = transform.position;
    //   marker_a_tm.rotation = glm::quatLookAt(axis, marker_a_tm.rotation * glm::vec3(0, 1, 0));
    //   marker_a_tm.scale = {0.2, 0.2, 2 + angle};
    // }
    // // {
    // //   marker_a_tm.position = tug_point + transform.position + tug_out_component / 2.f;
    // //   marker_a_tm.rotation = glm::quatLookAt(glm::normalize(tug_out_component), marker_a_tm.rotation * glm::vec3(0, 1, 0));
    // //   marker_a_tm.scale = {0.2, 0.2, glm::length(tug_out_component) / 2.f};
    // // }
    // {
    //   marker_b_tm.position = tug_point + transform.position + tug / 2.f;
    //   marker_b_tm.rotation = glm::quatLookAt(glm::normalize(tug), marker_b_tm.rotation * glm::vec3(0, 1, 0));
    //   marker_b_tm.scale = {0.2, 0.2, glm::length(tug) / 2};
    // }
    // // {
    // //   marker_b_tm.position = transform.position + axis * (1 + angle);
    // //   marker_b_tm.rotation = glm::quatLookAt(axis, marker_b_tm.rotation * glm::vec3(0, 1, 0));
    // //   marker_b_tm.scale = {0.2, 0.2, 1 + angle};
    // // }
    // {
    //   marker_c_tm.position = tug_point + transform.position + angular_tug / 2.f;
    //   marker_c_tm.rotation = glm::quatLookAt(glm::normalize(angular_tug), marker_c_tm.rotation * glm::vec3(0, 1, 0));
    //   marker_c_tm.scale = {0.2, 0.2, glm::length(angular_tug) / 2.f};
    // }

    // std::cout << reflect(axis) << std::endl;
    // std::cout << reflect(angle) << std::endl << std::endl;
    if (motion) {
      // if (time->uptime() > Time::duration(2)) {
      //   motion->composeAngular3(axis, 0);
      // } else {
      //   motion->angular.w = 0;
      //   motion->composeAngular3(axis, angle);
      //   // motion->angular = {axis, angle * 4};
      // }
      motion->composeAngular4(axis, 256 * angle * time->delta_seconds());
      // motion->composeAngular3(axis, 16 * angle * time->delta_seconds());
      // motion->position /= 2;

    }
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

int main() {
  App app;
  // app.add_plugins(Engine<glWindow::Builder<cevy::engine::DeferredRenderer, editor::Editor>>());
  app.add_plugins(Engine<glWindow::Builder<cevy::engine::DeferredRenderer>>());
  // app.add_plugins(Engine<glWindow>());
  app.add_plugins(physics::PhysicsPlugin());
  app.init_resource<Raycaster>();
  app.add_systems<core_stage::Startup>(initial_setup);
  app.add_systems<core_stage::Update>(rotate_camera);
  app.add_systems<core_stage::Update>(move_camera);
  app.add_systems<core_stage::Update>(click_sys);
  app.run();
}
