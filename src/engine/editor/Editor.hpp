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
#include "input/input.hpp"
#include <optional>

namespace cevy {
namespace editor {

class EditorPreRender : public ecs::core_stage::after<engine::PreRenderStage> {};
class EditorRender : public ecs::core_stage::after<engine::RenderStage> {};
class EditorInput : public ecs::core_stage::before<input::InputStage> {};

class Editor : public glWindow::Module {
  public:
  Editor(glWindow &): viewport_pos(std::nullopt), viewport_size(std::nullopt) {}

  void init(glWindow &glwindow);
  void deinit(glWindow &);
  void build(cevy::ecs::App &app);

  static void pre_render(cevy::ecs::Resource<cevy::engine::Window> windower);
  static void render(cevy::ecs::Resource<cevy::engine::Window> windower);
  static void intercept_inputs(cevy::ecs::World &world,
    cevy::ecs::EventWriter<cevy::input::cursorMoved> cursorMoved,
    cevy::ecs::Resource<cevy::engine::Window> windower
  );

  GLuint texture;
  GLuint framebuffer;
  std::optional<ImVec2> viewport_pos;
  std::optional<ImVec2> viewport_size;
};

} // namespace editor
}; // namespace cevy
