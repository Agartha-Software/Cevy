/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Forward Renderer
*/

#pragma once

#include "Camera.hpp"
#include "Color.hpp"
#include "Handle.hpp"
#include "Mesh.hpp"
#include "PbrMaterial.hpp"
#include "ShaderProgram.hpp"
#include "Window.hpp"
#include "pipeline.hpp"
#include "rendering.hpp"
#include "glWindow.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class cevy::engine::ForwardRenderer : public glWindow::Module {
  struct pipeline : engine::pipeline {
    struct constants {
      inline static constexpr int lightCount = 15;
    };
    struct uniforms : engine::pipeline::uniforms {
      struct lighting : engine::pipeline::uniforms::lighting {
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
      };
    };
  };

  template <typename... T>
  using Query = ecs::Query<T...>;
  template <typename T>
  using Resource = ecs::Resource<T>;

  public:
  template <typename Windower = cevy::engine::Window::GenericWindow>
  ForwardRenderer(const Windower & /* win */) {}

  ForwardRenderer(ForwardRenderer &&other) {
    *this = std::move(other);
  }

  ForwardRenderer &operator=(ForwardRenderer &&other) {
    this->glfWindow = other.glfWindow;
    this->uboLights = other.uboLights;
    other.uboLights = 0;
    this->shaderProgram = std::move(other.shaderProgram);
    this->defaultMaterial = other.defaultMaterial;
    return *this;
  }

  ~ForwardRenderer() override {
  }

  void build(ecs::App &app) override {
    app.add_systems<RenderStage>(ForwardRenderer::render_system);
  }

  void init(glWindow &win) override;

  void deinit(glWindow &_win) override {
    glDeleteBuffers(1, &this->uboLights);
    this->uboLights = 0;
  }
  void static render_system(
      Resource<Window> win, Query<Camera> cams,
      Query<option<Transform>, Handle<Mesh>, option<Handle<PbrMaterial>>, option<Color>> models,
      Query<option<Transform>, cevy::engine::PointLight> lights, const cevy::ecs::World &world);

  protected:
  GLFWwindow *glfWindow;
  uint32_t uboLights = 0;
  std::unique_ptr<ShaderProgram> shaderProgram;

  PbrMaterial defaultMaterial;
};
