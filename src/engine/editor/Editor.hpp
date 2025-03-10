/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Editor
*/

#pragma once

#include "EditorWindow.hpp"
#include "Stage.hpp"
#include "ecs.hpp"
#include "engine.hpp"
#include "glWindow.hpp"
#include "imgui.h"
#include "input.hpp"
#include "GameWindow.hpp"
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
  Editor(glWindow &): cursorInViewport(std::nullopt), viewportPos(std::nullopt), viewportSize(std::nullopt) {
    windows.push_back(std::make_unique<LogWindow>("left"));
    windows.push_back(std::make_unique<LogWindow>("right"));
    windows.push_back(std::make_unique<LogWindow>("bottom"));
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
};

} // namespace editor
}; // namespace cevy
