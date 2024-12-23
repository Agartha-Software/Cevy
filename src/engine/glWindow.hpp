/*
** Agartha-Software, 2024
** C++evy
** File description:
** openGL window handling
*/

#include "Camera.hpp"
#include "Color.hpp"
#include "Handle.hpp"
#include "Model.hpp"
#include "PbrMaterial.hpp"
#include "Scheduler.hpp"
#include "ShaderProgram.hpp"
#include "Query.hpp"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include <memory>

namespace cevy::engine {
class Window;
}

struct Light {
	glm::vec4 position;
	glm::vec3 color;
	float radius;
};


struct Environment {
	std::vector<Light> lights;
	glm::vec3 ambientColor;
	glm::vec3 fog;
};

class glWindow {
  template <typename T>
  using Handle = cevy::engine::Handle<T>;

  template <typename T>
  using Resource = cevy::ecs::Resource<T>;
  template <typename... T>
  using Query = cevy::ecs::Query<T...>;


  using Camera = cevy::engine::Camera;
  using Transform = cevy::engine::Transform;
  using PbrMaterial = cevy::engine::PbrMaterial;
  using Color = cevy::engine::Color;
  using Model = cevy::engine::Model;

  public:
  glWindow(int width, int height);
  glWindow(glWindow&& rhs) noexcept {
    this->width = rhs.width;
    this->height = rhs.height;
    this->env = rhs.env;
    this->shaderProgram = rhs.shaderProgram;
    this->glfWindow = rhs.glfWindow;
    rhs.shaderProgram = nullptr;
    rhs.glfWindow = nullptr;
    this->defaultMaterial = rhs.defaultMaterial;

    glfwSetWindowUserPointer(this->glfWindow, this);
  }
  glWindow(const glWindow&) = delete;
  ~glWindow();
  bool open();
  bool close();
  static void render_system(Resource<cevy::engine::Window> win,Query<Camera> cams,
         Query<option<Transform>, Handle<Model>, option<Handle<PbrMaterial>>, option<Color>> models,
         cevy::ecs::EventWriter<cevy::ecs::AppExit> close);
  void render(Query<Camera> cams,
         Query<option<Transform>, Handle<Model>, option<Handle<PbrMaterial>>, option<Color>> models,
         cevy::ecs::EventWriter<cevy::ecs::AppExit> close);

  void setupEnv();

  protected:
  void updateSize(int width, int height);
  void keyInput(int key, int scancode, int action, int mods);
  void cursor(double xpos, double ypos);
  void mouseInput(int button, int action, int mods);
  bool init_context();
  bool unload_context();

  static glWindow *getFromWin(GLFWwindow *glfWindow) {
    return static_cast<glWindow *>(glfwGetWindowUserPointer(glfWindow));
  }

  int width;
  int height;
  Environment env;
  ShaderProgram* shaderProgram;
  GLFWwindow *glfWindow;

  PbrMaterial defaultMaterial;
};

namespace cevy::engine {
class Window {
public:
  Window(glWindow&& win) {
    this->window = std::make_shared<glWindow>(std::forward<glWindow>(win));
  }
  Window(int width, int height) {
    this->window = std::make_shared<glWindow>(width, height);
  }
  glWindow* operator->() {
    return this->window.get();
  }
  glWindow* get() {
    return this->window.get();
  }
  glWindow& operator*() {
    return *this->window;
  }
  const glWindow& operator*() const {
    return *this->window;
  }
protected:
  std::shared_ptr<glWindow> window;
};
}
