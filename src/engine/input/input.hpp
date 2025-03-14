/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Input
*/

#pragma once

#include "App.hpp"
#include "Plugin.hpp"

namespace cevy {
namespace input {

class InputPlugin : public ecs::Plugin {
  public:
  void build(ecs::App &app);
};

} // namespace input
}; // namespace cevy
