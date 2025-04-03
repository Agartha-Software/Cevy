/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Editor Default Windows
*/

#pragma once

#include "glWindow.hpp"

#include <string>

namespace cevy::editor {
class Editor;

class EditorWindow {
  public:
  virtual void render(cevy::editor::Editor &editor, glWindow &glwindow,
                      cevy::ecs::World &world) = 0;
  bool open;
  bool resizable;
  bool background;
  bool draggable;
  const bool menuActive;
  const std::string id;
  int imGuiWindowFlags;

  EditorWindow &operator=(EditorWindow &&rhs) = delete;      // can not assign to const members
  EditorWindow &operator=(const EditorWindow &rhs) = delete; // can not assign to const members
  EditorWindow(bool menuActive, const std::string &id, int imGuiWindowFlags = 0)
      : open(true), resizable(true), background(true), draggable(true), menuActive(menuActive),
        id(id), imGuiWindowFlags(imGuiWindowFlags) {}
};

class BasicWindow : public EditorWindow {
  public:
  BasicWindow(const std::string id) : EditorWindow(true, id) {}

  void render(cevy::editor::Editor &, glWindow &, cevy::ecs::World &) override {}
};
}; // namespace cevy::editor
