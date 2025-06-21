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

void setup(Resource<asset::AssetManager> asset_manager, cevy::ecs::Commands cmd) {
  auto plane_handle = asset_manager->add(primitives::plane(16, 8, 8), "plane.mesh");
  auto mat_white = asset_manager->add(PbrMaterial(), "white.material");

  cmd.spawn(SunLight{{1, 1 ,1}, }, Transform(0, 0, 15));
  cmd.spawn(Camera(), Transform(glm::vec3(0, -10, 5),
                                    glm::quat({glm::half_pi<float>() * 0.8, 0, 0}), glm::vec3(1)));
  cmd.spawn(Mesh::load("./assets/BowlingPins.obj"));
  cmd.spawn(Mesh::load("./assets/BowlingBall.obj"));
  cmd.spawn(plane_handle, mat_white, Color(0.8, 0.8, 1), Transform(), physics::RigidBody(INFINITY), physics::Collider::primitives::Quad({8, 8}));
}

int main() {
  App app;
  app.add_plugins(engine::Engine<glWindow::Builder<engine::DeferredRenderer>>());
  // app.add_plugins(Engine<glWindow>());
  app.add_plugins(physics::PhysicsPlugin());
  app.add_systems<core_stage::Startup>(setup);
  app.run();
    return 0;
}
