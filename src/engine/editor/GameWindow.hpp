/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Editor Game Windows
*/

#pragma once

#include "glWindow.hpp"
#include "EditorWindow.hpp"

#include <string>


namespace cevy::editor {

class GameWindow : public EditorWindow {
    public:
    GameWindow() {}

    void render(cevy::editor::Editor &editor, glWindow &glwindow) override;

    bool getMenuActive() override {
      return false;
    }

    const std::string getId() override {
      return "GameWindow";
    }
  };
};
