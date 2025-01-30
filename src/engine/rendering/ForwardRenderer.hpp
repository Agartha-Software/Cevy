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
#include "Model.hpp"
#include "PbrMaterial.hpp"
#include "ShaderProgram.hpp"
#include "Window.hpp"
#include "pipeline.hpp"
#include "rendering.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class cevy::engine::ForwardRenderer {
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
  template <typename Windower = cevy::engine::Window::generic_window>
  ForwardRenderer(const Windower & /* win */) {}
  void init();
  void static render_system(
      ForwardRenderer &self, Query<Camera> cams,
      Query<option<Transform>, Handle<Model>, option<Handle<PbrMaterial>>, option<Color>> models,
      Query<option<Transform>, cevy::engine::PointLight> lights, const cevy::ecs::World &world);

  protected:
  GLFWwindow *glfWindow;
  uint uboLights = 0;
  ShaderProgram *shaderProgram;

  PbrMaterial defaultMaterial;
};
