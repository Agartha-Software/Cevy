/*
** Agartha-Software, 2023
** C++evy
** File description:
** Commands
*/

#pragma once

#include "ecs.hpp"

class cevy::ecs::Command {
  public:
  static void apply(cevy::ecs::World &) {};
};
