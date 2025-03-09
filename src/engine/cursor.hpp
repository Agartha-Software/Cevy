/*
** Agartha-Software, 2023
** C++evy
** File description:
** Cursor
*/

#pragma once


#include "glx.hpp" // IWYU pragma: keep : glfw includes GL.h, which must be included only after the helpers in glx
#include <GLFW/glfw3.h>

namespace cevy::engine {

enum CursorState {
  normal = GLFW_CURSOR_NORMAL,
  hidden = GLFW_CURSOR_HIDDEN,
  disabled = GLFW_CURSOR_DISABLED
};

};
