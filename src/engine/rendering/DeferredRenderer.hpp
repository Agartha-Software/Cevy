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
#include "deferred/Billboard.hpp"
#include "deferred/GBuffers.hpp"
#include "deferred/ShadowMap.hpp"
#include "pipeline.hpp"
#include "rendering.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class cevy::engine::DeferredRenderer {
  struct pipeline : engine::pipeline {
    using unorm8 = uint8_t; /// unsigned normalized: 1.0 is mapped to 255 etc
                            /// https://www.khronos.org/opengl/wiki/Normalized_Integer
    struct gBuffers {
      // !todo: document that even though these structs are not zero-size,
      // they only serve as user guidance and are definition-structs
      /// light accumulation
      struct gRender {
        static constexpr uint32_t attachment = GL_COLOR_ATTACHMENT0;
        using Type = glm::vec4;
        glm::vec3 color;
        float alpha; // ?
      };
      struct gPostition {
        static constexpr uint32_t attachment = GL_COLOR_ATTACHMENT1;
        using Type = glm::vec4;
        glm::vec3 position;
        float distance;
      };
      struct gNormal {
        static constexpr uint32_t attachment = GL_COLOR_ATTACHMENT2;
        using Type = glm::vec4;
        glm::vec3 normal;
        float unspecified; // metalness ?
      };
      struct gAlbedo {
        static constexpr uint32_t attachment = GL_COLOR_ATTACHMENT3;
        using Type = glm::vec<4, unorm8>;
        glm::vec<3, unorm8> color;
        unorm8 unspecified;
        // unorm8 roughness; /// inverse of exponent - 1
      };
      struct gSpecular {
        static constexpr uint32_t attachment = GL_COLOR_ATTACHMENT4;
        using Type = glm::vec<4, unorm8>;
        glm::vec<3, unorm8> color;
        unorm8 roughness; /// inverse of exponent - 1
      };
    };
    struct uniforms : engine::pipeline::uniforms {
      /// mvp for the object acting as a canvas in the deferred rendering passes
      struct canvas {
        using Type = glm::mat4;
        inline static constexpr auto name = "canvas";
      };
    };
  };

  template <typename... T>
  using Query = ecs::Query<T...>;
  template <typename T>
  using Resource = ecs::Resource<T>;

  public:
  template <typename Windower>
  DeferredRenderer(const Windower &win)
      : width(win.size().x), height(win.size().y), gbuffer(width, height), shadowMap() {
    this->aspect = float(width) / float(height);
    std::cout << " <<<< DeferredRenderer(win) @" << this << " <<<<" << std::endl;
  }

  DeferredRenderer(DeferredRenderer &&rhs) : gbuffer(std::forward<GBuffers &&>(rhs.gbuffer)) {
    std::cout << " <<<< DeferredRenderer MOVE CONSTRUCT @" << this << " <<<<" << std::endl;
    this->width = rhs.width;
    this->height = rhs.height;
    this->defaultMaterial = std::move(rhs.defaultMaterial);
    this->null_shader.swap(rhs.null_shader);
    // this->gBuffer_shader.swap(rhs.gBuffer_shader);
    this->compose_shader.swap(rhs.compose_shader);
    this->accumulate_shader.swap(rhs.accumulate_shader);
    // this->principled_shader.swap(rhs.principled_shader);
    rhs.alive = "DeferredRenderer is moved-from";
    this->primitives.sphere = std::move(rhs.primitives.sphere);
    this->primitives.blank = std::move(rhs.primitives.blank);
    this->primitives.flat = std::move(rhs.primitives.flat);
    this->shadowMap = std::move(rhs.shadowMap);
  }

  ~DeferredRenderer() {
    std::cout << this->alive << std::endl;
    std::cout << " <<<< ~DeferredRenderer @" << this << "<<<<" << std::endl;
  }

  void init();
  static void render_system(
      DeferredRenderer &self, Query<Camera> cams,
      Query<option<Transform>, Handle<Model>, option<Handle<PbrMaterial>>, option<Color>> models,
      Query<option<Transform>, option<cevy::engine::PointLight>, option<cevy::engine::SpotLight>>
          lights,
      const ecs::World &world);

  protected:
  void light_pass(const pipeline::Light &light);

  GLFWwindow *glfWindow;
  std::unique_ptr<ShaderProgram> null_shader = nullptr;
  // std::unique_ptr<ShaderProgram> gBuffer_shader = nullptr;
  // std::unique_ptr<ShaderProgram> principled_shader = nullptr;
  std::unique_ptr<ShaderProgram> accumulate_shader = nullptr;
  std::unique_ptr<ShaderProgram> compose_shader = nullptr;
  PbrMaterial defaultMaterial;

  std::string alive = "DeferredRenderer is uninitialized";

  int width;
  int height;
  float aspect;

  GBuffers gbuffer;
  Billboard billboard;

  ShadowMap shadowMap;

  struct {
    glm::mat4 view;
    glm::mat4 invView;
    std::vector<std::tuple<Handle<Model>, glm::mat4, uint16_t>> models;
  } renderContext;

  struct {
    Model sphere;
    Model cube;
    Texture blank;
    Texture black;
    Texture flat;
  } primitives;
};
