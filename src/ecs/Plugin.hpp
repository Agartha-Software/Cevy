/*
** Agartha-Software, 2023
** C++evy
** File description:
** Plugin
*/

#pragma once

#include "ecs.hpp"

class cevy::ecs::Plugin {
  public:
  virtual void build(cevy::ecs::App &) = 0;
  Plugin() = default;
  ~Plugin() = default;
};

namespace cevy::ecs {
class NullPlugin : public cevy::ecs::Plugin {
  void build(cevy::ecs::App &) override {};
};
}
