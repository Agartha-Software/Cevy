/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Editor
*/

#pragma once

#include "EditorWindow.hpp"
#include "GameWindow.hpp"
#include "LoggingWindow.hpp"
#include "ProfilingWindow.hpp"
#include "Stage.hpp"
#include "ecs.hpp"
#include "engine.hpp"
#include "glWindow.hpp"
#include "imgui.h"
#include "input.hpp"
#include <memory>
#include <optional>
#include <vector>

namespace cevy {
namespace editor {

class EditorPreRender : public ecs::core_stage::after<engine::PreRenderStage> {};
class EditorRender : public ecs::core_stage::after<engine::RenderStage> {};
class EditorInput : public ecs::core_stage::before<input::InputStage> {};

class Editor : public glWindow::Module {
  public:
  Editor(glWindow &)
      : cursorInViewport(std::nullopt), viewportPos(std::nullopt), viewportSize(std::nullopt) {
    windows.push_back(std::make_unique<ProfilingWindow>());
    windows.push_back(std::make_unique<LoggingWindow>());
    windows.push_back(std::make_unique<BasicWindow>("Basic"));
    windows.push_back(std::make_unique<GameWindow>());
  }

  void init(glWindow &glwindow);
  void deinit(glWindow &);
  void build(cevy::ecs::App &app);

  std::vector<std::unique_ptr<EditorWindow>> windows;
  GLuint texture;
  GLuint framebuffer;
  std::optional<bool> cursorInViewport;
  std::optional<ImVec2> viewportPos;
  std::optional<ImVec2> viewportSize;
  private:
  static void pre_render(cevy::ecs::World &world, cevy::ecs::Resource<cevy::engine::Window> windower);
  static void render(cevy::ecs::Resource<cevy::engine::Window> windower);
};

} // namespace editor
}; // namespace cevy
