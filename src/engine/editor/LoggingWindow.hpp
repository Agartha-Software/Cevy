/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Editor Logging Windows
*/

#pragma once

#include "glWindow.hpp"
#include "EditorWindow.hpp"

#include <string>


namespace cevy::editor {

class LoggingWindow : public EditorWindow {
    public:
    LoggingWindow() : EditorWindow(true, "Logger") {
    }

    void render(cevy::editor::Editor &editor, glWindow &glwindow, cevy::ecs::World &world) override;
  };
};
