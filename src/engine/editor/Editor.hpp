/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Editor
*/

#pragma once

#include "Plugin.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace cevy {
namespace editor {

class EditorPlugin : public ecs::Plugin {
  public:
  void build(ecs::App &app);
};

class Editor {
  public:

};

}
};
