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

int initial_setup(Resource<Asset<cevy::engine::Model>> mesh_manager,
                  Resource<Asset<cevy::engine::PbrMaterial>> material_manager, Commands cmd) {
  auto plane_handle = mesh_manager->load(primitives::plane(7, 4, 4));
  auto bunny = primitives::sphere(1, 32, 16);
  bunny.setModelMatrix(glm::mat4(
      Transform(0, 0, 0.5)));

  auto bunny_handle = mesh_manager->load(std::move(bunny));
  auto mat_white = material_manager->load(PbrMaterial());
  auto h_mat_bunny = material_manager->load(PbrMaterial(glm::vec3(0.1, .1, .1), glm::vec3(1), 12));
  h_mat_bunny->ambient = glm::vec3(0.07, 0.005, 0);
  // auto bunny = cmd.spawn(Transform(), bunny_handle, h_mat_bunny, Color(0, 0, 1));
  cmd.spawn(cevy::engine::Camera(),
            cevy::engine::Transform(glm::vec3(0, -10, 5),
                                    glm::quat({glm::half_pi<float>() * 0.8, 0, 0}), {1, 1, 1}));

  // std::cout << "added bunny...:" << bunny.id() << std::endl;

  auto rotator =
      cmd.spawn(bunny_handle, h_mat_bunny, Color(0, 0, 1),
                Transform(glm::vec3(0, 0, 0), glm::quat({0, 0, 0}), glm::vec3(1, 1, 1)),
                TransformVelocity(Transform(glm::vec3(0.5, 0, 0), glm::quat({0, 0, DEG2RAD * 90}),
                                            glm::vec3(1, 1, 1))));
  std::cout << "added rotator...:" << rotator.id() << std::endl;

  auto plane = cmd.spawn(plane_handle, mat_white, Color(0.8, 0.8, 1),
                         Transform(glm::vec3(0, 0, 0), glm::quat({0, 0, 0}), glm::vec3(1, 1, 1)));

  const int ringCount = 9;
  const float ringRadius = 5;
  for (int i = 0; i < ringCount; i++) {
    glm::vec3 rgb = 10.f * hsv2rgb({float(i) / ringCount, 0.9, 1.0f});
    auto h_mat_light = material_manager->load(PbrMaterial(glm::vec3(), glm::vec3(0), 1));
    h_mat_light->emit = rgb;
    Transform tm = Transform(
        glm::vec3(ringRadius * std::cos(glm::two_pi<float>() * float(i) / ringCount),
                  ringRadius * std::sin(glm::two_pi<float>() * float(i) / ringCount), 3.0f),
        glm::quat({0, 0, 0}), glm::vec3(.5, .5, .5));
    PointLight light = {rgb, 1.0f};
    auto entity = cmd.spawn(Parent{rotator.id()}, tm, light, bunny_handle, h_mat_light,
                            TransformVelocity(glm::quat({0, 2, -1})));
  }

  return 0;
}

int main() {
  App app;
  app.init_resource<AssetManager>();
  app.add_plugins(Engine<glWindow, cevy::engine::DeferredRenderer>());
  // app.add_plugins(Engine<glWindow, cevy::engine::ForwardRenderer>());
  app.add_systems<cevy::ecs::core_stage::PostStartup>(initial_setup);
  app.run();
}
