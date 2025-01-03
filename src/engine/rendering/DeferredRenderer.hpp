/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Deferred Renderer
*/

#pragma once

#include "Camera.hpp"
#include "Color.hpp"
#include "Handle.hpp"
#include "Model.hpp"
#include "PbrMaterial.hpp"
#include "ShaderProgram.hpp"
#include "rendering.hpp"
#include <GLFW/glfw3.h>
#include "pipeline.hpp"
#include <glm/glm.hpp>

class cevy::engine::DeferredRenderer {
  struct pipeline : engine::pipeline {
    using unorm8 = uint8_t; /// unsigned normalized: 1.0 is mapped to 255 etc https://www.khronos.org/opengl/wiki/Normalized_Integer
    struct gBuffers {
      struct gPostition {
        static constexpr uint attachment = GL_COLOR_ATTACHMENT0;
        using Type = glm::vec4;
        glm::vec3 position;
        float distance;
      };
      struct gNormal {
        static constexpr uint attachment = GL_COLOR_ATTACHMENT1;
        using Type = glm::vec4;
        glm::vec3 normal;
        float unspecified; // metalness ?
      };
      struct gAlbedo {
        static constexpr uint attachment = GL_COLOR_ATTACHMENT2;
        using Type = glm::vec<4, unorm8>;
        glm::vec<3, unorm8> color;
        unorm8 roughness; /// inverse of exponent - 1
      };
      struct gEmit {
        static constexpr uint attachment = GL_COLOR_ATTACHMENT3;
        using Type = glm::vec<4, unorm8>;
        glm::vec<3, unorm8> color;
        union {
          unorm8 value;
          enum MODE {
            _,
            halfLambert, /// enable halfLambert
            ambient, /// treat emit as additionnal ambient instead
          } flags;
        } mode; /// subject to change
      };
    };
  };

  template <typename... T>
  using Query = ecs::Query<T...>;
  template <typename T>
  using Resource = ecs::Resource<T>;

  public:
  template<typename Windower = cevy::engine::Window::generic_window>
  DeferredRenderer(const Windower& win) {
    auto size = win.size();
    this->width = size.x;
    this->height = size.y;
  }
  void init();
  void
  render(Query<Camera> cams,
         Query<option<Transform>, Handle<Model>, option<Handle<PbrMaterial>>, option<Color>> models,
         Query<option<Transform>, cevy::engine::PointLight> lights);
  protected:
  GLFWwindow *glfWindow;
  struct Environment {
    glm::vec3 ambientColor;
    glm::vec3 fog;
  } env;
  ShaderProgram *gBuffer_shader;
  ShaderProgram *principled_shader;
  PbrMaterial defaultMaterial;

  int width;
  int height;

  uint uboLights = 0;
  uint gBuffer;
  uint rbo;
  uint gPosition, gNormal, gAlbedo, gEmit;
  uint gAttachments[4];
};
