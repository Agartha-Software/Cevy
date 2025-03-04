/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Editor
*/

#pragma once

#include "Stage.hpp"
#include "engine.hpp"
#include "glWindow.hpp"

namespace cevy {
namespace editor {

class EditorPreRender : public ecs::core_stage::after<engine::PreRenderStage> {};
class EditorRender : public ecs::core_stage::after<engine::RenderStage> {};

class Editor : public glWindow::Module {
  public:
  Editor(glWindow &) {}

  void init(glWindow &glwindow);
  void deinit(glWindow &);
  void build(cevy::ecs::App &app);

  static void pre_render(cevy::ecs::Resource<cevy::engine::Window> windower);
  static void render(cevy::ecs::Resource<cevy::engine::Window> windower);

  GLuint texture;
  GLuint framebuffer;
};

} // namespace editor
}; // namespace cevy
