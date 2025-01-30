/*
** Agartha-Software, 2023
** C++evy
** File description:
** GameEngine
*/

#pragma once

#include "AssetManager.hpp"
#include "Atmosphere.hpp"
#include "Camera.hpp"
#include "Color.hpp"
#include "DefaultPlugin.hpp"
#include "ForwardRenderer.hpp"
#include "Line.hpp"
#include "PhysicsProps.hpp"
#include "Plugin.hpp"
#include "Stage.hpp"
#include "Target.hpp"
#include "Transform.hpp"
#include "Velocity.hpp"
#include "Window.hpp"
#include "ecs.hpp"
#include "engine.hpp"
#include "glWindow.hpp"
#include "App.hpp"
#include "input/input.hpp"

namespace cevy::engine {
template <template <typename T> typename Windower = glWindow, typename Renderer = ForwardRenderer>
class Engine : public cevy::ecs::Plugin {
  public:
  // void build(cevy::ecs::App &app);

  void build(cevy::ecs::App &app) {
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
    app.init_resource<cevy::engine::Atmosphere>();
    app.init_resource<cevy::engine::Window>(Windower<Renderer>(1280, 720));
    app.init_component<cevy::engine::Camera>();
    app.init_component<cevy::engine::Velocity>();
    app.init_component<cevy::engine::PhysicsProps>();
    app.init_component<cevy::engine::Target>();
    app.init_component<cevy::engine::Line>();
    app.init_component<cevy::engine::Parent>();
    app.init_component<cevy::engine::Transform>();
    app.init_component<cevy::engine::TransformVelocity>();
    app.init_component<cevy::engine::PointLight>();
    app.init_component<cevy::engine::Color>();
    // app.init_component<cevy::engine::Atmosphere>();
    app.add_plugins(cevy::engine::AssetManagerPlugin());
    app.add_plugins(cevy::input::InputPlugin());
    app.add_systems<cevy::engine::PreRenderStage>(update_camera);
    app.add_systems<cevy::engine::RenderStage>(Windower<Renderer>::render_system);
    app.add_systems<ecs::core_stage::PostUpdate>(TransformVelocity::system);
    app.add_systems<cevy::ecs::core_stage::PreUpdate>(Transform::children_system);
  };
};
} // namespace cevy::engine
