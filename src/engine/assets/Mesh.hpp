/*
** Agartha-Software, 2023
** C++evy
** File description:
** Mesh
*/

#pragma once

#include "raylib.h"

namespace cevy::engine {
class Mesh {
  public:
  Model mesh;

  Mesh(const Model &mesh) : mesh(mesh){};
};
} // namespace cevy::engine
