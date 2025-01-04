/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Forward Renderer
*/

#pragma once

#include "Atmosphere.hpp"
#include "Camera.hpp"
#include "Color.hpp"
#include "Handle.hpp"
#include "Model.hpp"
#include "PbrMaterial.hpp"
#include "ShaderProgram.hpp"
#include "Window.hpp"
#include "rendering.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

struct Environment {
  glm::vec3 ambientColor;
  glm::vec3 fog;
};

class cevy::engine::ForwardRenderer {
  template <typename... T>
  using Query = ecs::Query<T...>;
  template <typename T>
  using Resource = ecs::Resource<T>;

  public:
  template<typename Windower = cevy::engine::Window::generic_window>
  ForwardRenderer(const Windower& /* win */) {}
  void init();
  void
  render(Query<Camera> cams,
         Query<option<Transform>, Handle<Model>, option<Handle<PbrMaterial>>, option<Color>> models,
         Query<option<Transform>, cevy::engine::PointLight> lights,
         std::optional<ref<Atmosphere>> atmosphere);
  protected:
  GLFWwindow *glfWindow;
  uint uboLights = 0;
  Environment env;
  ShaderProgram *shaderProgram;

  PbrMaterial defaultMaterial;
};
