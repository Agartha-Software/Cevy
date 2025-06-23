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
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/ext/quaternion_float.hpp>

namespace cevy {
namespace engine {

template <typename Windower>
class Engine;

class Transform;
class Motion;

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
inline std::string reflect<glm::vec2>(const glm::vec2 &v) {
    return "glm::vec2 {" + reflect(v.x) + ", " + reflect(v.y) + " }";
}

template <>
inline std::string reflect<glm::vec3>(const glm::vec3 &v) {
  return "glm::vec3 {" + reflect(v.x) + ", " + reflect(v.y) + ", " + reflect(v.z) + " }";
}
template <>
inline std::string reflect<glm::vec4>(const glm::vec4 &v) {
  return "glm::vec3 {" + reflect(v.x) + ", " + reflect(v.y) + ", " + reflect(v.z) + ", " + reflect(v.w) + " }";

}
template <>
inline std::string reflect<glm::quat>(const glm::quat &v) {
  return "glm::quat {" + reflect(v.x) + ", " + reflect(v.y) + ", " + reflect(v.z) + ", " + reflect(v.w) + " }";
}
template <>
inline std::string reflect<glm::mat4>(const glm::mat4 &v) {
    return "glm::mat4 {\n"
    + reflect(v[0]) + "\n"
    + reflect(v[1]) + "\n"
    + reflect(v[2]) + "\n"
    + reflect(v[3]) + "\n"
    + "}";
}

/// calculate a normalized vector with its length stored in its last component
template<int Size, typename T, ::glm::qualifier Q>
::glm::vec<Size + 1, T, Q> homogenous(const ::glm::vec<Size, T, Q> &v) {
  T w = ::glm::length(v);
  return {v / w, w};
}

} // namespace cevy

namespace glm {
using vec4u8 = vec<4, uint8_t>;
}
