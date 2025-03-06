/*
** Agartha-Software, 2023
** C++evy
** File description:
** Engine
*/

#include "Engine.hpp"
#include "App.hpp"
#include "AssetManager.hpp"
#include "Camera.hpp"
#include "ClearColor.hpp"
#include "Color.hpp"
#include "DefaultPlugin.hpp"
#include "EntityCommands.hpp"
#include "Event.hpp"
#include "Line.hpp"
#include "PhysicsProps.hpp"
#include "Target.hpp"
#include "Transform.hpp"
#include "Velocity.hpp"
#include "ecs.hpp"
#ifdef DEBUG
#include "imgui.h"
#endif

#include "rendering.hpp"
#include "rlImGui.h"

void init_window() {
#ifdef DEBUG
  set_trace_log_level(LOG_ALL);
#else
  set_trace_log_level(LOG_ERROR);
#endif
  set_config_flags(FLAG_WINDOW_RESIZABLE);
  InitWindow(800, 450, "raylib [core] example - basic window");
  SetTargetFPS(60);
  rlImGuiSetup(true);
}

void close_game(cevy::ecs::EventWriter<cevy::ecs::AppExit> close) {
  if (WindowShouldClose())
    close.send(cevy::ecs::AppExit{});
}

void update_window(cevy::ecs::Query<cevy::engine::Camera> cams, cevy::ecs::World &w,
#ifdef DEBUG
                   cevy::ecs::Resource<cevy::engine::DebugWindow> debug,
#endif
                   cevy::ecs::Resource<cevy::engine::ClearColor> clearcolor) {
  ClearBackground(clearcolor.get());
  for (auto [cam] : cams) {
    BeginMode3D(cam);
    render_models(w);
    render_lines(w);
    EndMode3D();
#ifdef DEBUG
    rl_im_gui_begin();
    if (!ImGui::Begin("Debug Informations", &(debug.get().open))) {
      ImGui::End();
    } else {
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      ImGui::Text("Number of entities: %zu", w.entities().valid_size());
      ImGui::End();
    }
    rlImGuiEnd();
#endif
  }
  EndDrawing();
}

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
  app.init_component<cevy::engine::Camera>();
  app.init_component<cevy::engine::Velocity>();
  app.init_component<cevy::engine::PhysicsProps>();
  app.init_component<cevy::engine::Target>();
  app.init_component<cevy::engine::Line>();
  app.init_component<cevy::engine::Transform>();
  app.init_component<cevy::engine::TransformVelocity>();
  app.init_component<cevy::engine::Color>();
  app.init_component<cevy::engine::ClearColor>();
  app.add_plugins(cevy::engine::AssetManagerPlugin());
  app.add_systems<cevy::engine::PreStartupRenderStage>(init_window);
  app.add_systems<cevy::engine::PreRenderStage>(close_game);
  app.add_systems<cevy::engine::PreRenderStage>(update_camera);
  app.add_systems<cevy::engine::RenderStage>(update_window);
  app.add_systems<ecs::core_stage::Update>(TransformVelocity::system);
}

/*

void cevy::Engine::DebugWindow (void) {
    if (IsKeyPressed(KEY_F10)) {
        debugMode = (debugMode == false);
        if (debugMode)
            EnableCursor();
    }
    if(debugMode) {
    } else {
        DisableCursor();
    }
}

void cevy::Engine::Fullscreen (void) {
    if (IsWindowFullscreen()) {
        int display = GetCurrentMonitor();
        this->screenWidth = GetMonitorWidth(display);
        this->screenHeight = GetMonitorHeight(display);
    } else {
        this->screenWidth = GetScreenWidth();
        this->screenHeight = GetScreenHeight();
    }
    if (!IsKeyPressed(KEY_F11))
        return;
    SetWindowSize(this->screenWidth, this->screenWidth);
        ToggleFullscreen();
}
*/
