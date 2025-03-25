/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Editor Logging Windows
*/

#include "LoggingWindow.hpp"
#include "Editor.hpp"
#include "imgui.h"

void cevy::editor::LoggingWindow::render(cevy::editor::Editor &, glWindow &, cevy::ecs::World &) {
  ImGui::Text("1");
}
