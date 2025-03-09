/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Input
*/

#pragma once

#include "App.hpp"
#include "Plugin.hpp"
#include "Stage.hpp"

namespace cevy {
namespace input {

class InputStage : public ecs::core_stage::before<ecs::core_stage::PreUpdate> {};

class InputPlugin : public ecs::Plugin {
  public:
  void build(ecs::App &app);
};

} // namespace input
}; // namespace cevy
