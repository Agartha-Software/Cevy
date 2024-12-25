/*
** Agartha-Software, 2023
** C++evy
** File description:
** Engine
*/

#include "Engine.hpp"
#include "Stage.hpp"
#include "App.hpp"
#include "AssetManager.hpp"
#include "Camera.hpp"
#include "ClearColor.hpp"
#include "Color.hpp"
#include "ecs/DefaultPlugin.hpp"
#include "Line.hpp"
#include "PhysicsProps.hpp"
#include "Target.hpp"
#include "Transform.hpp"
#include "Velocity.hpp"
#include "ecs.hpp"
#include "glWindow.hpp"

#ifdef DEBUG
#include "imgui.h"
#endif

// #include "rlImGui.h"
// #include <imgui_impl_glfw.h>


void cevy::engine::Engine::build(cevy::ecs::App &app) {
  app.add_plugins(cevy::ecs::DefaultPlugin());
  app.add_stage<StartupRenderStage>();
  app.add_stage<PreStartupRenderStage>();
  app.add_stage<PostStartupRenderStage>();
  app.add_stage<RenderStage>();
  app.add_stage<PreRenderStage>();
  app.add_stage<PostRenderStage>();
#ifdef DEBUG
  app.init_resource<cevy::engine::DebugWindow>(cevy::engine::DebugWindow{.open = true});
#endif
  app.init_resource<cevy::engine::ClearColor>(cevy::engine::Color(255, 255, 255));
  app.init_resource<Window>(1280, 720);
  app.init_component<cevy::engine::Camera>();
  app.init_component<cevy::engine::Velocity>();
  app.init_component<cevy::engine::PhysicsProps>();
  app.init_component<cevy::engine::Target>();
  app.init_component<cevy::engine::Line>();
  app.init_component<cevy::engine::Transform>();
  app.init_component<cevy::engine::TransformVelocity>();
  app.init_component<cevy::engine::PointLight>();
  app.init_component<cevy::engine::Color>();
  app.init_component<cevy::engine::ClearColor>();
  app.add_plugins(cevy::engine::AssetManagerPlugin());
  app.add_systems<cevy::engine::PreRenderStage>(update_camera);
  app.add_systems<cevy::engine::RenderStage>(glWindow::render_system);
  app.add_systems<ecs::core_stage::Update>(TransformVelocity::system);
}
