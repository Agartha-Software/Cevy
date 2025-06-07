#include "Collider.hpp"
#include "Physics.hpp"
#include "RigidBody.hpp"
#include <glm/ext/quaternion_transform.hpp>
#include <optional>
#include <type_traits>
#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/trigonometric.hpp>

#include "App.hpp"
#include "AssetManager.hpp"
#include "Assets.hpp"
#include "Color.hpp"
#include "DeferredRenderer.hpp"
#include "EnginePlugin.hpp"
#include "EntityCommands.hpp"
#include "ForwardRenderer.hpp"
#include "Mesh.hpp"
#include "PbrMaterial.hpp"
#include "Transform.hpp"
#include "Velocity.hpp"
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

struct Target {};

struct Raycaster {
  physics::Collider hitPlane = physics::Collider(physics::Shape::Plane({}, {0, 0, 1}));
};

// template<typename Data, typename ComponentSrc, typename ComponentDst>
struct Driver {
  friend class DriverServer;
  Entity driver;
  Entity driven;
  std::function<void(Entity, Entity, World &)> func;

  template <typename Data, typename SrcComponent, typename DestComponent, typename F>
  static Driver Builder(Entity driver, Entity driven, const SrcComponent &source_component,
                        const Data &source_data, const DestComponent &dest_component,
                        const Data &dest_data, F functor) {
    static_assert(std::is_same_v<std::invoke_result_t<F, Data>, Data> ||
                      std::is_same_v<F, std::nullopt_t>,
                  "Functor must be Data -> Data or nullopt");
    // static_assert(reinterpret_cast<const char *>(&source_component) + sizeof(SrcComponent) >
    //                   reinterpret_cast<const char *>(&source_data),
    //               "source data must be a member in source component reference");
    // static_assert(reinterpret_cast<const char *>(&dest_component) + sizeof(SrcComponent) >
    //                   reinterpret_cast<const char *>(&dest_data),
    //               "destination data must be a member in destination component reference");
    ptrdiff_t offset_source = reinterpret_cast<const char *>(&source_data) -
                              reinterpret_cast<const char *>(&source_component);
    ptrdiff_t offset_dest = reinterpret_cast<const char *>(&dest_data) -
                            reinterpret_cast<const char *>(&dest_component);
    Driver built = {
        .driver = driver,
        .driven = driven,
        .func = [&functor, offset_dest, offset_source](Entity driver, Entity driven, World &world) {
          Data *destination = reinterpret_cast<Data *>(
              reinterpret_cast<char *>(&world.get_components<DestComponent>()[driven].value()) +
              offset_dest);
          Data *source = reinterpret_cast<Data *>(
              reinterpret_cast<char *>(&world.get_components<SrcComponent>()[driver].value()) +
              offset_source);
          if (std::is_same_v<F, std::nullopt_t>) {
            *destination = *source;
          } else {
            *destination = functor(*source);
          }
        }};
    return built;
  }

  private:
};

struct DriverServer {
  public:
  std::list<Driver> drivers;
  static void system(Resource<DriverServer> self, World &world) {
    for (const auto &driver : self->drivers) {
      driver.func(driver.driver, driver.driven, world);
    }
  }
};

int initial_setup(Resource<Assets<Mesh>> mesh_manager,
                  Resource<Assets<PbrMaterial>> material_manager, Resource<Atmosphere> atmosphere,
                  Resource<DriverServer> drivers, Resource<Time> time, Commands cmd) {
  atmosphere->ambiant = {atmosphere->ambiant.r * 2, atmosphere->ambiant.g * 2,
                         atmosphere->ambiant.b * 2};
  atmosphere->fog = {atmosphere->fog.r * 2, atmosphere->fog.g * 2, atmosphere->fog.b * 2};

  auto plane_handle = mesh_manager->add(primitives::plane(32, 4, 4));
  auto sphere = primitives::cube(1);
  // sphere.setModelMatrix(glm::mat4(Transform(0, 0, 1)));

  auto sphere_handle = mesh_manager->add(std::move(sphere));
  auto mat_white = material_manager->add(PbrMaterial());
  mat_white->roughness = 0.002;
  auto mat_sphere = material_manager->add(PbrMaterial(glm::vec3(0.1, .1, .1), glm::vec3(1), 0.5));
  cmd.spawn(Camera(), Transform(glm::vec3(0, -10, 5),
                                glm::quat({glm::half_pi<float>() * 0.8, 0, 0}), glm::vec3(1)));

  auto rotator = cmd.spawn(Transform(), TransformVelocity(glm::quat({0, 0, DEG2RAD * 90})));

  auto x = cmd.spawn(Transform(0, 0, 1));
  auto y = cmd.spawn(Transform(), Parent {x.id()});
  auto z = cmd.spawn(Transform(), Parent {y.id()});
  auto cube = cmd.spawn(Parent {z.id()}, ::Target(), sphere_handle, mat_sphere, Color(1, 0.7, 1),
                        Transform(), TransformVelocity());

  auto a = cmd.spawn(sphere_handle, mat_sphere, Color(1, 0.7, 1),
                     Transform({0, 0, 1.5}, glm::quat({0, glm::radians(-45.f), 0.f}), {1, 1, 1}));

  auto c = cmd.spawn(sphere_handle, mat_sphere, Color(1, 0.7, 1),
                     Transform({0, 0, 10.5},
                               glm::rotate(glm::quat({0.f, 0.f, 0.f}), glm::radians(45.f),
                                           glm::normalize(glm::vec3(0.f, 1.f, 1.f))),
                               {1, 1, 1}));

  auto controller_x =
      cmd.spawn(sphere_handle, mat_white, physics::Collider(physics::Shape::Sphere({}, 1)),
                Transform({4, 0, 0}, glm::quat({0, 0, 0}), {0.1, 0.1, 0.1}));
  // auto controller_y = cmd.spawn(sphere_handle, mat_white,
  // physics::Collider(physics::Shape::Sphere({}, 1)), Transform({0, 4, 0}, glm::quat({0, 0, 0}),
  // {0.1, 0.1, 0.1})); auto controller_z = cmd.spawn(sphere_handle, mat_white,
  // physics::Collider(physics::Shape::Sphere({}, 1)), Transform({0, 6, 0}, glm::quat({0, 0, 0}),
  // {0.1, 0.1, 0.1}));

  const Transform t_ref;
  drivers->drivers.push_back(Driver::Builder(
      controller_x.id(), x.id(), t_ref, t_ref.position.y, t_ref, t_ref.rotation.x, [](float x) {
        glm::quat a({0, glm::radians(-45.f), 0});
        glm::quat b = glm::rotate(glm::quat({0.f, 0.f, 0.f}), glm::radians(45.f),
                                  glm::normalize(glm::vec3(0.f, 1.f, 1.f)));

        return glm::slerp(a, b, std::min(1.f, std::max(0.f, x))).x;
      }));
  drivers->drivers.push_back(Driver::Builder(
      controller_x.id(), x.id(), t_ref, t_ref.position.y, t_ref, t_ref.rotation.y, [](float x) {
        glm::quat a({0, glm::radians(-45.f), 0});
        glm::quat b = glm::rotate(glm::quat({0.f, 0.f, 0.f}), glm::radians(45.f),
                                  glm::normalize(glm::vec3(0.f, 1.f, 1.f)));

        return glm::slerp(a, b, std::min(1.f, std::max(0.f, x))).y;
      }));
  drivers->drivers.push_back(Driver::Builder(
      controller_x.id(), x.id(), t_ref, t_ref.position.y, t_ref, t_ref.rotation.z, [](float x) {
        glm::quat a({0, glm::radians(-45.f), 0});
        glm::quat b = glm::rotate(glm::quat({0.f, 0.f, 0.f}), glm::radians(45.f),
                                  glm::normalize(glm::vec3(0.f, 1.f, 1.f)));

        return glm::slerp(a, b, std::min(1.f, std::max(0.f, x))).z;
      }));
  drivers->drivers.push_back(Driver::Builder(
      controller_x.id(), x.id(), t_ref, t_ref.position.y, t_ref, t_ref.rotation.w, [](float x) {
        glm::quat a({0, glm::radians(-45.f), 0});
        glm::quat b = glm::rotate(glm::quat({0.f, 0.f, 0.f}), glm::radians(45.f),
                                  glm::normalize(glm::vec3(0.f, 1.f, 1.f)));

        return glm::slerp(a, b, std::min(1.f, std::max(0.f, x))).w;
      }));

  drivers->drivers.push_back(Driver::Builder(controller_x.id(), x.id(), t_ref, t_ref.position.y,
                                             t_ref, t_ref.position.z, [](float x) {
                                               return 10.5f * std::min(1.f, std::max(0.f, x)) +
                                                      1.5f * (1 - std::min(1.f, std::max(0.f, x)));
                                             }));

  // drivers->drivers.push_back(Driver::Builder(controller_y.id(), y.id(), t_ref, t_ref.position.x,
  // t_ref, t_ref.rotation.x, [](float x){ return glm::quat({0, x, 0}).x;}));
  // drivers->drivers.push_back(Driver::Builder(controller_y.id(), y.id(), t_ref, t_ref.position.x,
  // t_ref, t_ref.rotation.y, [](float y){ return glm::quat({0, y, 0}).y;}));
  // drivers->drivers.push_back(Driver::Builder(controller_y.id(), y.id(), t_ref, t_ref.position.x,
  // t_ref, t_ref.rotation.z, [](float z){ return glm::quat({0, z, 0}).z;}));
  // drivers->drivers.push_back(Driver::Builder(controller_y.id(), y.id(), t_ref, t_ref.position.x,
  // t_ref, t_ref.rotation.w, [](float w){ return glm::quat({0, w, 0}).w;}));

  // drivers->drivers.push_back(Driver::Builder(controller_z.id(), z.id(), t_ref, t_ref.position.x,
  // t_ref, t_ref.rotation.x, [](float x){ return glm::quat({0, 0, x}).x;}));
  // drivers->drivers.push_back(Driver::Builder(controller_z.id(), z.id(), t_ref, t_ref.position.x,
  // t_ref, t_ref.rotation.y, [](float y){ return glm::quat({0, 0, y}).y;}));
  // drivers->drivers.push_back(Driver::Builder(controller_z.id(), z.id(), t_ref, t_ref.position.x,
  // t_ref, t_ref.rotation.z, [](float z){ return glm::quat({0, 0, z}).z;}));
  // drivers->drivers.push_back(Driver::Builder(controller_z.id(), z.id(), t_ref, t_ref.position.x,
  // t_ref, t_ref.rotation.w, [](float w){ return glm::quat({0, 0, w}).w;}));

  cmd.spawn(Parent {cube.id()}, Color(1, 0, 0), sphere_handle, mat_white,
            Transform({0, 0, 0}, glm::quat({0, 0, 0}), {3, 0.1, 0.1}));
  cmd.spawn(Parent {cube.id()}, Color(0, 1, 0), sphere_handle, mat_white,
            Transform({0, 0, 0}, glm::quat({0, 0, 0}), {0.1, 3, 0.1}));
  cmd.spawn(Parent {cube.id()}, Color(0, 0, 1), sphere_handle, mat_white,
            Transform({0, 0, 0}, glm::quat({0, 0, 0}), {0.1, 0.1, 3}));

  cmd.spawn(plane_handle, mat_white, Color(0.8, 0.8, 1), Transform());

  const int ringCount = 3;
  const float ringRadius = 10;
  // for (int i = 0; i < ringCount; i++) {
  //   glm::vec3 rgb = 1000.f * hsv2rgb({float(i) / ringCount, 0.9, 1.0f});
  //   auto mat_light = material_manager->add(PbrMaterial(glm::vec3(), glm::vec3(), 1));
  //   mat_light->emit = rgb;
  //   glm::vec3 pos = glm::vec3(ringRadius * std::cos(glm::two_pi<float>() * float(i) / ringCount),
  //   ringRadius * std::sin(glm::two_pi<float>() * float(i) / ringCount), 15 + 0 *float(i) /
  //   ringCount); glm::quat rot = glm::quatLookAt(-glm::normalize(pos), {0, 0, 1}); Transform tm =
  //   Transform(pos, rot, glm::vec3(.5, .5, .5)); SpotLight light = {rgb, 0.8, 0.5};
  //   // PointLight light = {rgb, 1.0f};
  //   auto entity = cmd.spawn(Parent {rotator.id()}, tm, light, sphere_handle, mat_light);
  // }

  glm::vec3 pos = glm::vec3(0, ringRadius, 10);
  glm::quat rot = glm::quatLookAt(-glm::normalize(pos), {0, 0, 1});
  Transform tm = Transform(pos, rot, glm::vec3(.5, .5, .5));
  SunLight light = {{1.3, 1.2, 0.9}, 30, 30};
  auto entity = cmd.spawn(tm, light);

  return 0;
}

void click_sys(Query<Transform, option<TransformVelocity>> transforms, Commands cmd,
               Query<const Camera, const Transform> camera,
               Query<Entity, Transform, physics::Collider, option<physics::RigidBody>> colliders,
               Resource<input::ButtonInput<input::MouseButton>> mouse_buttons,
               Resource<input::ButtonInput<input::KeyCode>> keyboard,
               Resource<asset::AssetManager> asset_manager,
               cevy::ecs::Resource<cevy::input::cursorPosition> cursorPosition,
               cevy::ecs::Resource<Raycaster> raycaster, cevy::ecs::Resource<Window> window,
               Resource<Time> time) {
  auto o_camera = camera.get_single();
  static std::optional<Entity> o_ent = {};
  static float ent_mass;

  if (!o_ent && mouse_buttons->is_pressed(input::MouseButton::Left) && o_camera) {
    auto [camera, cam_trans] = o_camera.value();
    auto window_size = window->windowSize();
    glm::vec2 screen_space = {(float(cursorPosition->pos.x) / window_size.x) * 2.f - 1.f,
                              ((float(cursorPosition->pos.y) / window_size.y) * -2.f + 1.f) /
                                  window_size.x * window_size.y};
    // screen_space = glm::inverse(camera.projection) * glm::vec4(screen_space, 1, 0);
    glm::mat4 cam_tm = glm::mat4(cam_trans);
    cam_tm /= cam_tm[3][3];
    auto origin = cam_tm * glm::vec4(0, 0, 0, 1);
    auto direction = glm::inverse(camera.projection) * glm::vec4(screen_space, 1, 1);
    direction = cam_tm * glm::vec4(direction.xyz(), 0);
    origin /= origin.w;
    auto ray = physics::Ray {origin, glm::normalize(direction.xyz())};

    std::vector<std::pair<physics::Collision, Entity>> collisions;
    for (auto [en, tm, collider, body] : colliders) {
      if ((o_ent && o_ent.value() == en) || (body && body->iMass == 0))
        continue;
      collisions.push_back({collider.raycast(tm, ray), en});
    }

    auto collision = physics::Ray::filter(ray, collisions.begin(), collisions.end());

    if (collision == collisions.end() || !collision->first.hit) {
      return;
    }
    o_ent.emplace(collision->second);
    auto body = std::get<option<physics::RigidBody> &>(colliders.get(collision->second).value());
    if (body) {
      ent_mass = body->iMass;
      body->iMass = 0;
    }
  }
  if (mouse_buttons->is_just_released(input::MouseButton::Left) && o_ent) {
    auto body = std::get<option<physics::RigidBody> &>(colliders.get(o_ent.value()).value());
    if (body) {
      body->iMass = ent_mass;
    }
    auto [_, vel] = transforms.get(o_ent.value()).value();
    if (vel) {
      vel->animated = false;
    }
    o_ent.reset();
  }
  if (o_ent) {
    auto [camera, cam_trans] = o_camera.value();
    auto window_size = window->windowSize();
    glm::vec2 screen_space = {(float(cursorPosition->pos.x) / window_size.x) * 2.f - 1.f,
                              ((float(cursorPosition->pos.y) / window_size.y) * -2.f + 1.f) /
                                  window_size.x * window_size.y};
    // screen_space = glm::inverse(camera.projection) * glm::vec4(screen_space, 1, 0);
    glm::mat4 cam_tm = glm::mat4(cam_trans);
    cam_tm /= cam_tm[3][3];
    auto origin = cam_tm * glm::vec4(0, 0, 0, 1);
    auto direction = glm::inverse(camera.projection) * glm::vec4(screen_space, 1, 1);
    direction = cam_tm * glm::vec4(direction.xyz(), 0);
    origin /= origin.w;
    auto ray = physics::Ray {origin, glm::normalize(direction.xyz())};

    auto [transform, vel] = transforms.get(o_ent.value()).value();

    glm::mat4 plane_tm = glm::translate(glm::mat4(1), transform.position);
    if (keyboard->is_pressed(input::KeyCode::Space)) {
      plane_tm = plane_tm * glm::mat4(cam_trans.rotation);
    }

    auto collision = raycaster->hitPlane.raycast(plane_tm, ray);
    if (vel) {
      vel->animated = true;
      vel->position += (collision.location - transform.position) / float(time->delta_seconds());
      vel->position /= 2;
    }
    if (collision.hit)
      transform.position = collision.location;
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
        // std::cout << "motion!" << mouse_motion.delta.value().x << " " <<
        // (mouse_motion.delta.value().y) << std::endl;
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
  app.init_component<::Target>();
  // app.add_plugins(Engine<glWindow::Builder<cevy::engine::ForwardRenderer>>());
  app.init_resource<Raycaster>();
  app.init_resource<DriverServer>();
  app.add_plugins(Engine<glWindow::Builder<cevy::engine::DeferredRenderer>>());
  app.add_plugins(physics::PhysicsPlugin());
  app.add_systems<core_stage::PostStartup>(initial_setup);
  // app.add_systems<core_stage::Update>(rotate_cube);
  app.add_systems<core_stage::PostUpdate>(DriverServer::system);
  app.add_systems<core_stage::Update>(rotate_camera);
  app.add_systems<core_stage::Update>(move_camera);
  app.add_systems<core_stage::Update>(rotate_camera);
  app.add_systems<core_stage::Update>(click_sys);
  app.run();
}
