/*
** Agartha-Software, 2023
** C++evy
** File description:
** default pipeline definition
*/

#pragma once

#include "PointLight.hpp"
#include <glm/glm.hpp>

namespace cevy::engine::pipeline {
struct Light;
/// represents glsl sampler2D
struct sampler2D {};

namespace constants {
constexpr int lightCount = 15;
}

namespace uniforms {
/// model transform matrix
struct model {
  using Type = glm::mat4;
  inline static constexpr auto name = "model";
};
/// model normal transform matrix
struct model_normal {
  using Type = glm::mat3;
  inline static constexpr auto name = "model_normal";
};
/// view transform matrix, including camera projection
struct view {
  using Type = glm::mat4;
  inline static constexpr auto name = "view";
};
/// inverse view transform matrix, including camera projection
struct invView {
  using Type = glm::mat4;
  inline static constexpr auto name = "invView";
};

namespace environment {
struct fog {
  using Type = glm::vec3;
  inline static constexpr auto name = "fog";
};
struct fog_far {
  using Type = float;
  inline static constexpr auto name = "fog_far";
};
} // namespace environment

namespace lighting {
/// number of lights with data specified
struct activeLights {
  using Type = int;
  inline static constexpr auto name = "activeLights";
};
struct LightBlock {
  using Type = int;
  inline static constexpr auto binding = 1;
  inline static constexpr auto name = "LightBlock";
  struct lights {
    using Type = Light[constants::lightCount];
  };
};
} // namespace lighting
namespace PbrMaterial {
struct albedo {
  using Type = glm::vec3;
  inline static constexpr auto name = "albedo";
};
struct specular_tint {
  using Type = glm::vec3;
  inline static constexpr auto name = "specular_tint";
};
struct phong_exponent {
  using Type = float;
  inline static constexpr auto name = "phong_exponent";
};
struct halflambert {
  using Type = bool;
  inline static constexpr auto name = "halflambert";
};
struct diffuseTexture {
  using Type = sampler2D;
  inline static constexpr auto name = "diffuseTexture";
};
} // namespace shading
} // namespace uniforms
namespace layout {
/// vertex position
struct vertexPosition {
  using Type = glm::vec4;
  inline static constexpr auto name = "vertexPosition";
  inline static constexpr auto location = 0;
};
/// per-vertex color
struct vertexColor {
  using Type = glm::vec3;
  inline static constexpr auto name = "vertexColor";
  inline static constexpr auto location = 1;
};
/// vertex normal
struct vertexNormal {
  using Type = glm::vec3;
  inline static constexpr auto name = "vertexNormal";
  inline static constexpr auto location = 2;
};
/// vertex texture coordinate: UVs
struct vertexTexCoord {
  using Type = glm::vec2;
  inline static constexpr auto name = "vertexTexCoord";
  inline static constexpr auto location = 3;
};
} // namespace layout

struct Light {
  inline static const int count = 15;
  template <glm::length_t N, typename T>
  Light(const PointLight &l, glm::vec<N, T> position) {
    this->position = {float(position.x), float(position.y), float(position.z), 1.0f};
    this->radius = l.radius;
    this->color = l.color;
  }
  Light(glm::vec4 position, glm::vec3 color, float radius)
      : position(position), color(color), radius(radius) {};
  glm::vec4 position;
  glm::vec3 color;
  float radius; /// 0 for directionnal, non-0 for point;
};
} // namespace cevy::engine::pipeline
