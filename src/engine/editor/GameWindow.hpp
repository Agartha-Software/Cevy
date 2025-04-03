/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Editor Game Windows
*/

#pragma once

#include "EditorWindow.hpp"
#include "glWindow.hpp"

#include <string>

namespace cevy::editor {

class GameWindow : public EditorWindow {
  public:
  GameWindow() : EditorWindow(false, "Game") {}

  void render(cevy::editor::Editor &editor, glWindow &glwindow, cevy::ecs::World &world) override;
};
}; // namespace cevy::editor
