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

using namespace cevy;
using namespace ecs;
using namespace engine;

struct Origin {
  Transform transform;
  Motion velocity;
};

struct ResetClock {
  double time;
};

struct Raycaster {
  physics::Collider hitPlane = physics::Collider::primitives::Quad({10, 10});
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
  // gravity->acceleration = {};
  time->reset();
  world->dragDensity *= 0.1;
  // time->setTimescale(0.2);

  auto plane_handle = asset_manager->add(primitives::plane(16, 8, 8), "plane.mesh");
  auto spring_handle = asset_manager->add(primitives::cube(1), "spring.mesh");
  spring_handle->setModelMatrix(glm::scale(glm::translate(glm::mat4(1), {0, 0, 0.5}), {0.05, 0.05, 0.5}));

  // auto plane_handle = mesh_manager->add();
  auto sphere = primitives::sphere(1, 32, 16);
  auto cube = primitives::cube(1);
  // sphere.setModelMatrix(glm::mat4(Transform(0, 0, 1.5)));

  auto sphere_handle = asset_manager->add(std::move(sphere), "sphere.mesh");
  auto cube_handle = asset_manager->add(std::move(cube), "cube.mesh");

  auto mat_white = asset_manager->add(PbrMaterial(), "white.material");
  auto mat_sphere = asset_manager->add(PbrMaterial(glm::vec3(0.8, 0.8, 0.8), glm::vec3(1), 12), "sphere.material");
  auto _camera =
      cmd.spawn(Camera(), Transform(glm::vec3(0, -10, 5),
                                    glm::quat({glm::half_pi<float>() * 0.8, 0, 0}), glm::vec3(1)));

  auto origin_a = Origin {
      Transform({2, 0, 3}, glm::quat({0, 0, 0,}), glm::vec3(.4)),
      Motion({-2, 0, 0}),
  };
  auto origin_b = Origin {
      Transform({-2, 0, 6}, glm::quat({1, 0.5, 0.2,}), glm::vec3(1)),
      Motion({2, 0, 0}),
  };
  auto sphere_a =
      cmd.spawn(sphere_handle, mat_sphere, Color(1, 0.1, 0.1), origin_a, origin_a.transform,
                origin_a.velocity, physics::RigidBody(.8), physics::Collider::primitives::Sphere(1));
  auto cube_b =
  cmd.spawn(cube_handle, mat_sphere, Color(0.1, 0.1, 1), origin_b, origin_b.transform,
            origin_b.velocity, physics::RigidBody(10), physics::Collider::primitives::Box({1, 1, 1}));
  cmd.spawn(plane_handle, mat_white, Color(0.8, 0.8, 1), Transform({0, 8, 8}, glm::quat({80 * DEG2RAD, 0, 0}), {1, 1, 1}), physics::RigidBody(INFINITY), physics::Collider::primitives::Quad({8, 8}));
  cmd.spawn(plane_handle, mat_white, Color(0.8, 0.8, 1), Transform(), physics::RigidBody(INFINITY), physics::Collider::primitives::Quad({8, 8}));
  // cmd.spawn(PointLight{{100, 100 ,100}, 1, 30}, Transform(0, 0, 10));
  // cmd.spawn(SpotLight{{100, 100 ,100}, 0.2, 0.8}, Transform(0, 0, 15));
  cmd.spawn(SunLight{{1, 1 ,1}, }, Transform(0, 0, 15));

  auto anchor = cmd.spawn(Transform({0, 0, 5}, glm::quat({0, 0, 0}), {0.3, 0.3, 0.3}), physics::RigidBody::Passive(), sphere_handle, mat_white);

  // cmd.spawn(physics::Spring{anchor.id(), sphere_a.id(), 0.1, 0.2}, Transform(), spring_handle, mat_white);
  // cmd.spawn(physics::Spring{anchor.id(), cube_b.id(), 0.1, 0.2}, Transform(), spring_handle, mat_white);
  return 0;
}

void click_sys(
  Query<Transform, option<Motion>> transforms, Commands cmd, Query<const Camera, const Transform> camera,
  Query<Entity, Transform, physics::Collider, physics::RigidBody> colliders,
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

  if (mouse_buttons->is_just_pressed(input::MouseButton::Left) && o_camera) {
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
      if ((o_ent && o_ent.value() == en) || body.iMass == 0)
        continue;
      collisions.push_back({collider.raycast(tm, ray), en});
    }

    auto collision = physics::Ray::filter(ray, collisions.begin(), collisions.end());

    if (collision == collisions.end() || !collision->first.hit) {
      return;
    }
    o_ent.emplace(collision->second);
    auto body = std::get<physics::RigidBody&>(colliders.get(collision->second).value());
    ent_mass = body.iMass;
    body.iMass = 0;
  }
  if (mouse_buttons->is_just_released(input::MouseButton::Left) && o_ent) {
    std::get<physics::RigidBody&>(colliders.get(o_ent.value()).value()).iMass = ent_mass;
    auto [_, vel] = transforms.get(o_ent.value()).value();
    if (vel) {
      vel->animated = false;
    }
    o_ent.reset();
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

    auto [transform, vel] = transforms.get(o_ent.value()).value();


    glm::mat4 plane_tm = glm::translate(glm::mat4(1), transform.position);
    if (keyboard->is_pressed(input::KeyCode::Space)) {
      plane_tm = plane_tm * glm::mat4(cam_trans.rotation);
    }

    auto collision = raycaster->hitPlane.raycast(plane_tm, ray);
    if (vel) {
      vel->animated = true;
      vel->linear += (collision.location - transform.position) / float(time->delta_seconds());
      vel->linear /= 2;
    }
    if (collision.hit)
      transform.position = collision.location;
  }
}

void click_sys2(
  Query<Transform> transforms, Commands cmd, Query<const Camera, const Transform> camera,
  Query<Entity, Transform, physics::Collider> colliders,
  Resource<input::ButtonInput<input::MouseButton>> mouse_buttons,
  Resource<asset::AssetManager> asset_manager,
  cevy::ecs::Resource<cevy::input::cursorPosition> cursorPosition,
  cevy::ecs::Resource<Window> window
) {
  static int once = false;
  static std::optional<Entity> o_ent = {};
  if (camera.size() == 0 && !once) {
    once = true;
    return;
  }
  auto o_camera = camera.get_single();

  if (mouse_buttons->is_pressed(input::MouseButton::Left) && o_camera) {
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

    std::vector<physics::Collision> collisions;
    for (auto [en, tm, collider] : colliders) {
      if (o_ent && o_ent.value() == en)
        continue;
      collisions.push_back(collider.raycast(tm, ray));
    }

    auto collision = physics::Ray::filter(ray, collisions.begin(), collisions.end());

    if (!collision.hit) {
      return;
    }
    if (!o_ent) {
      o_ent.emplace(cmd.spawn(physics::RigidBody(INFINITY), physics::Collider(physics::Shape::Sphere({}, 0.5)), Transform(collision.location, glm::identity<glm::quat>(), glm::vec3(3)), asset_manager->get<Mesh>("sphere.mesh").value(), asset_manager->get<PbrMaterial>("sphere.material").value()).id());
    } else {
      std::get<Transform&>(transforms.get(o_ent.value()).value()).position = collision.location;
    }
  }
}

void pos_reset(Resource<Time> time,
               Query<Entity, Transform, Motion> transforms,
               Resource<input::ButtonInput<input::KeyCode>> keyboard,
               Query<Entity, const Origin> origins) {
  // for (auto [tm, _vel] : transforms) {
  //   if (std::abs(tm.position.x) > 20) {
  //     tm.position.x = 0;
  //   }
  //   if (std::abs(tm.position.y) > 20) {
  //     tm.position.y = 0;
  //   }
  //   if (std::abs(tm.position.z) > 20) {
  //     tm.position.z = 0;
  //   }
  // }
  if (keyboard->is_just_pressed(input::KeyCode::R)) {
    for (auto [entity, origin] : origins) {
      const auto &got = transforms.get(entity);
      auto &[en, tm, vel] = got.value();
      tm.position = origin.transform.position;
      vel.linear = origin.velocity.linear;
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
  static glm::vec2 rotation = {0.f, glm::pi<float>() * 0.3f};

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
  app.init_component<Origin>();
  app.init_resource<ResetClock>();
  app.init_resource<Raycaster>();
  app.add_systems<core_stage::Startup>(initial_setup);
  app.add_systems<core_stage::Update>(rotate_camera);
  app.add_systems<core_stage::Update>(move_camera);
  app.add_systems<core_stage::Update>(pos_reset);
  app.add_systems<core_stage::Update>(click_sys);
  app.run();
}
