/*
** Agartha-Software, 2023
** C++evy
** File description:
** Diffuse
*/

#pragma once

#include "raylib.h"

namespace cevy::engine {
class Diffuse {
  public:
  Texture2D texture;

  Diffuse(const Texture2D &texture) : texture(texture){};
};
} // namespace cevy::engine
