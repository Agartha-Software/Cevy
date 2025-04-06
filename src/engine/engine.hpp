/*
** Agartha-Software, 2023
** C++evy
** File description:
** Engine declarations
*/

#pragma once

#include "Stage.hpp"
#include "cevy.hpp"
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>

namespace cevy {
namespace engine {

template <typename Windower>
class Engine;

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

} // namespace engine
template <>
inline std::string reflect<glm::vec3>(const glm::vec3 &i);
template <>
inline std::string reflect<glm::vec4>(const glm::vec4 &i);
template <>
inline std::string reflect<glm::quat>(const glm::quat &i);
template <>
inline std::string reflect<glm::mat4>(const glm::mat4 &i);
} // namespace cevy

namespace glm {
using vec4u8 = vec<4, uint8_t>;
}
