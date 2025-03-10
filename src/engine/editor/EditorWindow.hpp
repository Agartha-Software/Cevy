/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Editor Default Windows
*/

#pragma once

#include "imgui.h"
#include "glWindow.hpp"

#include <string>

namespace cevy::editor {
  class Editor;

  class EditorWindow {
    public:
    virtual void render(cevy::editor::Editor &editor, glWindow &glwindow) = 0;
    virtual bool getMenuActive() = 0;
    virtual const std::string getId() = 0;
    bool enabled;

    EditorWindow(): enabled(false) {
    }
  };

  class LogWindow : public EditorWindow {
    const std::string id;

    public:
    LogWindow(const std::string id) : id(id) {}

    void render(cevy::editor::Editor &editor, glWindow &glwindow) override {
      ImGui::Text("Test1");
      ImGui::Text("Test2");
      ImGui::Text("Test3");
    }

    bool getMenuActive() override {
      return true;
    }

    const std::string getId() override {
      return id;
    }
  };
};
