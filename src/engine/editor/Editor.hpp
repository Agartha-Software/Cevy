/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Editor
*/

#pragma once

#include "Stage.hpp"
#include "ecs.hpp"
#include "engine.hpp"
#include "glWindow.hpp"
#include "imgui.h"
#include "input.hpp"
#include <optional>

namespace cevy {
namespace editor {

class EditorPreRender : public ecs::core_stage::after<engine::PreRenderStage> {};
class EditorRender : public ecs::core_stage::after<engine::RenderStage> {};
class EditorInput : public ecs::core_stage::before<input::InputStage> {};

class Editor : public glWindow::Module {
  public:
  Editor(glWindow &): cursorInViewport(std::nullopt), viewportPos(std::nullopt), viewportSize(std::nullopt) {}

  void init(glWindow &glwindow);
  void deinit(glWindow &);
  void build(cevy::ecs::App &app);

  GLuint texture;
  GLuint framebuffer;
  std::optional<bool> cursorInViewport;
  std::optional<ImVec2> viewportPos;
  std::optional<ImVec2> viewportSize;
};

} // namespace editor
}; // namespace cevy
