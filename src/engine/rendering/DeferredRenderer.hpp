/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Deferred Renderer
*/

#pragma once

#include "Atmosphere.hpp"
#include "Camera.hpp"
#include "Color.hpp"
#include "Handle.hpp"
#include "Model.hpp"
#include "PbrMaterial.hpp"
#include "ShaderProgram.hpp"
#include "deferred/Billboard.hpp"
#include "deferred/GBuffers.hpp"
#include "pipeline.hpp"
#include "rendering.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class cevy::engine::DeferredRenderer {
  struct pipeline : engine::pipeline {
    using unorm8 = uint8_t; /// unsigned normalized: 1.0 is mapped to 255 etc
                            /// https://www.khronos.org/opengl/wiki/Normalized_Integer
    struct gBuffers {
      struct gRender {
        static constexpr uint attachment = GL_COLOR_ATTACHMENT0;
        using Type = glm::vec4;
        glm::vec3 color;
        float alpha; // ?
      };
      struct gPostition {
        static constexpr uint attachment = GL_COLOR_ATTACHMENT1;
        using Type = glm::vec4;
        glm::vec3 position;
        float distance;
      };
      struct gNormal {
        static constexpr uint attachment = GL_COLOR_ATTACHMENT2;
        using Type = glm::vec4;
        glm::vec3 normal;
        float unspecified; // metalness ?
      };
      struct gAlbedo {
        static constexpr uint attachment = GL_COLOR_ATTACHMENT3;
        using Type = glm::vec<4, unorm8>;
        glm::vec<3, unorm8> color;
        unorm8 unspecified;
        // unorm8 roughness; /// inverse of exponent - 1
      };
      struct gSpecular {
        static constexpr uint attachment = GL_COLOR_ATTACHMENT4;
        using Type = glm::vec<4, unorm8>;
        glm::vec<3, unorm8> color;
        unorm8 roughness; /// inverse of exponent - 1
      };
      // struct gEmit {
      //   static constexpr uint attachment = GL_COLOR_ATTACHMENT5;
      //   using Type = glm::vec<4, unorm8>;
      //   glm::vec<3, unorm8> color;
      //   union {
      //     unorm8 value;
      //     enum MODE {
      //       _,
      //       halfLambert, /// enable halfLambert
      //       ambient,     /// treat emit as additionnal ambient instead
      //     } flags;
      //   } mode; /// subject to change
      // };
    };
  };

  template <typename... T>
  using Query = ecs::Query<T...>;
  template <typename T>
  using Resource = ecs::Resource<T>;

  public:
  template <typename Windower>
  DeferredRenderer(const Windower &win)
      : width(win.size().x), height(win.size().y), gbuffer(width, height) {
    this->aspect = float(width) / float(height);
    std::cout << " <<<< DeferredRenderer(win) @" << this << " <<<<" << std::endl;
  }

  DeferredRenderer(DeferredRenderer &&rhs) : gbuffer(std::forward<GBuffers &&>(rhs.gbuffer)) {
    std::cout << " <<<< DeferredRenderer MOVE CONSTRUCT @" << this << " <<<<" << std::endl;
    this->width = rhs.width;
    this->height = rhs.height;
    this->defaultMaterial = rhs.defaultMaterial;
    this->gBuffer_shader.swap(rhs.gBuffer_shader);
    this->compose_shader.swap(rhs.compose_shader);
    this->accumulate_shader.swap(rhs.accumulate_shader);
    // this->principled_shader.swap(rhs.principled_shader);
    this->uboLights = rhs.uboLights;
    rhs.alive = "DeferredRenderer is moved-from";
    rhs.uboLights = 0;
  }

  ~DeferredRenderer() {
    std::cout << this->alive << std::endl;
    std::cout << " <<<< ~DeferredRenderer @" << this << "<<<<" << std::endl;
  }

  void init();
  static void
  render(DeferredRenderer& self, Query<Camera> cams,
         Query<option<Transform>, Handle<Model>, option<Handle<PbrMaterial>>, option<Color>> models,
         Query<option<Transform>, cevy::engine::PointLight> lights, const ecs::World &world);

  protected:
  GLFWwindow *glfWindow;
  std::unique_ptr<ShaderProgram> gBuffer_shader = nullptr;
  // std::unique_ptr<ShaderProgram> principled_shader = nullptr;
  std::unique_ptr<ShaderProgram> accumulate_shader = nullptr;
  std::unique_ptr<ShaderProgram> compose_shader = nullptr;
  PbrMaterial defaultMaterial;

  std::string alive = "DeferredRenderer is uninitialized";

  int width;
  int height;
  float aspect;

  uint uboLights = 0;

  GBuffers gbuffer;
  Billboard billboard;
};
