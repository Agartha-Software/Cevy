/*
** Agartha-Software, 2023
** C++evy
** File description:
** Engine declarations
*/

#pragma once

#include "Stage.hpp"

namespace cevy::engine {

class AssetManager;

#ifdef DEBUG
struct DebugWindow {
  bool open;
};
#endif

class StartupRenderStage : public cevy::ecs::core_stage::after<cevy::ecs::core_stage::PreStartup> {
};
class PreStartupRenderStage : public cevy::ecs::core_stage::before<StartupRenderStage> {};
class PostStartupRenderStage : public cevy::ecs::core_stage::after<StartupRenderStage> {};

class RenderStage : public cevy::ecs::core_stage::after<cevy::ecs::core_stage::PostUpdate> {};
class PreRenderStage : public cevy::ecs::core_stage::before<RenderStage> {};
class PostRenderStage : public cevy::ecs::core_stage::after<RenderStage> {};
} // namespace cevy::engine
