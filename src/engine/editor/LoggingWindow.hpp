/*
** AgarthaSoftware, 2025
** Cevy
** File description:
** Editor Logging Windows
*/

#pragma once

#include "EditorWindow.hpp"
#include "glWindow.hpp"

#include <string>

namespace cevy::editor {

class LoggingWindow : public EditorWindow {
  public:
  LoggingWindow() : EditorWindow(true, "Logger") {}

  void render(cevy::editor::Editor &editor, glWindow &glwindow, cevy::ecs::World &world) override;
};
}; // namespace cevy::editor
