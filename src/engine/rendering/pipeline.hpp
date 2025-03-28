/*
** Agartha-Software, 2023
** C++evy
** File description:
** default pipeline definition
*/

#pragma once

#define GLM_FORCE_SWIZZLE

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>

#include "PointLight.hpp"
#include "ShaderProgram.hpp"
#include "SpotLight.hpp"
#include "Transform.hpp"

namespace cevy::engine {
struct pipeline {
  struct Light;
  /// represents glsl sampler2D
  struct sampler2D {};

  struct uniforms {
    enum class NormalMode : int {
      None = 0b00,
      Tangeant = 0b01,
      Mesh = 0b11,
    };

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

    struct environment {
      struct fog {
        using Type = glm::vec3;
        inline static constexpr auto name = "fog";
      };
      struct fog_far {
        using Type = float;
        inline static constexpr auto name = "fog_far";
      };
    }; // struct environment

    struct lighting {
      /// multiplier of light energy in the compositor
      struct exposure {
        using Type = float;
        inline static constexpr auto name = "exposure";
      };
    }; // struct lighting
    struct pbrMaterial {
      // diffuse constant
      struct diffuse {
        using Type = glm::vec3;
        inline static constexpr auto name = "diffuse_const";
      };
      // specular constant
      struct specular {
        using Type = glm::vec3;
        inline static constexpr auto name = "specular_const";
      };
      // roughness constant
      struct roughness {
        using Type = float;
        inline static constexpr auto name = "roughness_const";
      };
      // emission constant
      struct emit {
        using Type = glm::vec3;
        inline static constexpr auto name = "emit_const";
      };
      // lambertian method
      struct halflambert {
        using Type = bool;
        inline static constexpr auto name = "halflambert";
      };
      // lambertian method
      struct normal_mode {
        using Type = NormalMode;
        inline static constexpr auto name = "normal_mode";
      };
      /// non-physically based additionnal ambient
      struct custom_ambient {
        using Type = glm::vec3;
        inline static constexpr auto name = "custom_ambient";
      };
      struct diffuse_texture {
        using Type = sampler2D;
        glm::vec3 color;
        float alpha;
        inline static constexpr auto name = "diffuse_texture";
        inline static constexpr auto binding = 0;
      };
      struct emission_texture {
        using Type = sampler2D;
        glm::vec3 color;
        float unspecified;
        inline static constexpr auto name = "emission_texture";
        inline static constexpr auto binding = 2;
      };
      struct shader_generic {
        // use metallic
        struct metallic {
          using Type = bool;
          inline static constexpr auto name = "metallic";
        };
        struct specular_texture {
          using Type = sampler2D;
          glm::vec3 color;
          float roughness;
          inline static constexpr auto name = "specular_texture";
          inline static constexpr auto binding = 1;
        };
      };
      struct shader_pbr {
        struct shading_texture {
          using Type = sampler2D;
          float metallness;
          float ior;
          float anisitropy;
          float roughness;
          inline static constexpr auto name = "specular_texture";
          inline static constexpr auto binding = 1;
        };
      };
    }; // struct PbrMaterial
  }; // struct uniforms

  struct layout {
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
  }; // struct layout

  struct Instance {
    glm::mat4 matrix;
    glm::vec4 color;
  };

  struct Light {
    enum class Type : uint32_t {
      Point = 1,
      Spot = 2,
      Sun = 3,
    };

    inline static constexpr int count = 15;
    template <typename LightType>
    Light(const LightType &l, const Transform &tm) : Light(l, tm) {}
    Light(const PointLight &l, const Transform &tm) {
      // this->position = {float(position.x), float(position.y), float(position.z), 1.0f};
      this->model = {
          glm::vec4(1, 0, 0, 0),    //
          glm::vec4(0, 1, 0, 0),    //
          glm::vec4(0, 0, 1, 0),    //
          glm::vec4(tm.position, 1) //
      }; // collumn major, visually transposed
      this->radius = l.radius;
      this->color = l.color;
      this->range = l.range;
      this->range = l.range;
      this->falloff = 2;
      this->type = Type::Point;
    }
    Light(const SpotLight &l, const Transform &tm) {

      this->model = glm::translate(glm::mat4(1), tm.position) * glm::mat4(tm.rotation);

      this->radius = l.softness;
      this->angle = l.angle;
      this->range = l.range;
      this->falloff = 2;
      this->color = l.color;
      this->type = Type::Spot;
    }
    Light(const SunLight &l, const Transform &tm) {
      this->model = glm::translate(glm::mat4(1), tm.position) * glm::mat4(tm.rotation);

      this->radius = 0;
      this->angle = 0;
      this->range = l.range;
      this->falloff = 2;
      this->color = l.color;
      this->type = Type::Spot;
    }
    Light(glm::mat4 model, glm::vec3 color, float radius)
        : model(model), color(color), radius(radius) {};
    glm::mat4 model;
    glm::vec3 color;
    float radius;
    float range;
    float angle;
    float falloff;
    Type type;
  };
};
template <typename Pipeline = pipeline>
class ShaderBuilder {
  public:
  static ShaderProgram build_from_files(const std::string &vertex, const std::string &fragment);
  static ShaderProgram build_from_source(const std::string &vertex, const std::string &fragment);

  protected:
  template<typename>
  friend class ShaderBuilder;
  static void build(ShaderProgram &shader);
};

template <typename pipeline>
ShaderProgram ShaderBuilder<pipeline>::build_from_files(const std::string &vertex,
                                                        const std::string &fragment) {
  ShaderProgram shader;
  shader.initFromFiles(vertex, fragment);

  ShaderBuilder<pipeline>::build(shader);

  return shader;
}

template <typename pipeline>
ShaderProgram ShaderBuilder<pipeline>::build_from_source(const std::string &vertex,
                                                         const std::string &fragment) {
  ShaderProgram shader;
  shader.initFromStrings(vertex, fragment);

  ShaderBuilder<pipeline>::build(shader);

  return shader;
}
}; // namespace cevy::engine

template<>
void cevy::engine::ShaderBuilder<cevy::engine::pipeline>::build(ShaderProgram &shader); // pipeline.cpp
