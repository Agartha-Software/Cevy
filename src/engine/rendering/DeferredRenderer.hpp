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
#include "Mesh.hpp"
#include "PbrMaterial.hpp"
#include "ShaderProgram.hpp"
#include "Window.hpp"
#include "deferred/Billboard.hpp"
#include "deferred/GBuffers.hpp"
#include "deferred/ShadowMap.hpp"
#include "engine.hpp"
#include "glWindow.hpp"
#include "pipeline.hpp"
#include "rendering.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <memory>

class cevy::engine::DeferredRenderer : public glWindow::Module {
  template <typename... T>
  using Query = ecs::Query<T...>;
  template <typename T>
  using Resource = ecs::Resource<T>;

  public:
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

  DeferredRenderer(const glWindow &win)
      : glfWindow(win.getGLFWwindow()), width(win.renderSize.x), height(win.renderSize.y), gbuffer(width, height), shadowMap()  {
    this->aspect = float(width) / float(height);
    std::cout << " <<<< DeferredRenderer(win) @" << this << " <<<<" << std::endl;
  }

  DeferredRenderer(DeferredRenderer &&rhs) : gbuffer(std::forward<GBuffers &&>(rhs.gbuffer)) {
    std::cout << " <<<< DeferredRenderer MOVE CONSTRUCT @" << this << " <<<<" << std::endl;
    this->width = rhs.width;
    this->height = rhs.height;
    // this->defaultMaterial = std::move(rhs.defaultMaterial);
    this->defaultShader = std::move(rhs.defaultShader);
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

  ~DeferredRenderer() override {
    std::cout << this->alive << std::endl;
    std::cout << " <<<< ~DeferredRenderer @" << this << "<<<<" << std::endl;
  }

  void build(ecs::App &app) override {
    app.add_systems<RenderStage>(DeferredRenderer::render_system);
    app.resource<AssetManager>().add_factory<Shader>(
      "gbuffer_generic", std::function([]() {
        return ShaderBuilder<pipeline>::build_from_files(
            "assets/engine/shaders/simple.vert", "assets/engine/shaders/gbuffer_generic.frag");
      }));

    app.resource<AssetManager>().add_factory<Shader>(
      "gbuffer_pbr", std::function([]() {
        return ShaderBuilder<pipeline>::build_from_files(
            "assets/engine/shaders/simple.vert", "assets/engine/shaders/gbuffer_pbr.frag");
      }));
  }

  void init(glWindow &) override;
  void deinit(glWindow &) override;
  static void render_system(
      Resource<Window> win, Query<Camera> cams,
      Query<option<Transform>, Handle<Mesh>, option<Handle<PbrMaterial>>, option<Color>> models,
      Query<option<Transform>, option<cevy::engine::PointLight>, option<cevy::engine::SpotLight>,
            option<cevy::engine::SunLight>>
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
  // Handle<PbrMaterial> defaultMaterial = Handle<PbrMaterial>(PbrMaterial(), 0);
  std::unique_ptr<ShaderProgram> defaultShader = nullptr;

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
    // std::vector<std::tuple<Handle<Mesh>, glm::mat4, uint16_t>> models;
    // std::vector<std::tuple<Handle<Mesh>, glm::mat4, uint16_t>> models;
    std::unordered_map<std::tuple<Handle<Mesh>, Handle<PbrMaterial>>, std::vector<pipeline::Instance>> models;
  } renderContext;

  struct {
    Mesh sphere;
    Mesh cube;
    Texture blank;
    Texture black;
    Texture flat;
  } primitives;
};

template <>
void cevy::engine::ShaderBuilder<cevy::engine::DeferredRenderer::pipeline>::build(
    ShaderProgram &shader); // DeferredRender.cpp
