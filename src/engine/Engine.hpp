/*
** Agartha-Software, 2023
** Cevy
** File description:
** GameEngine
*/

#pragma once

#include "Plugin.hpp"
#include "Stage.hpp"
#include "ecs.hpp"

namespace cevy::engine {
class StartupRenderStage : public cevy::ecs::core_stage::after<cevy::ecs::core_stage::PreStartup> {
};
class PreStartupRenderStage : public cevy::ecs::core_stage::before<StartupRenderStage> {};
class PostStartupRenderStage : public cevy::ecs::core_stage::after<StartupRenderStage> {};

class RenderStage : public cevy::ecs::core_stage::after<cevy::ecs::core_stage::PostUpdate> {};
class PreRenderStage : public cevy::ecs::core_stage::before<RenderStage> {};
class PostRenderStage : public cevy::ecs::core_stage::after<RenderStage> {};

class Engine : public cevy::ecs::Plugin {
  public:
  void build(cevy::ecs::App &app);
};
} // namespace cevy::engine
