/*
** Agartha-Software, 2023
** C++evy
** File description:
** default pipeline definition
*/

#pragma once

#include "PointLight.hpp"
#include <glm/glm.hpp>

namespace cevy::engine {
struct pipeline {
  template <typename T>
  struct texture;
  struct Light;
  /// represents glsl sampler2D
  struct sampler2D {};

  struct uniforms {
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
    struct PbrMaterial {
      struct albedo {
        using Type = glm::vec3;
        inline static constexpr auto name = "albedo";
      };
      struct specular {
        using Type = glm::vec3;
        inline static constexpr auto name = "specular";
      };
      struct roughness {
        using Type = float;
        inline static constexpr auto name = "roughness";
      };
      struct emit {
        using Type = glm::vec3;
        inline static constexpr auto name = "emit";
      };
      struct halflambert {
        using Type = bool;
        inline static constexpr auto name = "halflambert";
      };
      /// non-physically based additionnal ambient
      struct custom_ambient {
        using Type = glm::vec3;
        inline static constexpr auto name = "custom_ambient";
      };
      struct diffuseTexture {
        using Type = sampler2D;
        inline static constexpr auto name = "diffuseTexture";
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

  struct Light {
    enum class Type {
      Point,
    };
    inline static const int count = 15;
    template <glm::length_t N, typename T>
    Light(const PointLight &l, glm::vec<N, T> position) {
      // this->position = {float(position.x), float(position.y), float(position.z), 1.0f};
      this->model = {
          glm::vec4(l.range, 0, 0, 0), //
          glm::vec4(0, l.range, 0, 0), //
          glm::vec4(0, 0, l.range, 0), //
          glm::vec4(position, 1)       //
      }; // collumn major, visually transposed
      this->radius = l.radius;
      this->range = l.range;
      this->color = l.color;
      this->type = Type::Point;
    }
    Light(glm::mat4 model, glm::vec3 color, float radius, Type type)
        : model(model), color(color), radius(radius), type(type) {};
    glm::mat4 model;
    glm::vec3 color;
    float radius; /// 0 for directionnal, non-0 for point;
    float range;
    Type type;
  };
};
}; // namespace cevy::engine
