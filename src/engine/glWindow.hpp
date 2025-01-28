/*
** Agartha-Software, 2024
** C++evy
** File description:
** openGL window handling
*/

#pragma once

#include "Window.hpp"
#if (_WIN32)
#include <GL/gl3w.h>
#endif
#if (__linux__)
#include <GL/glew.h>
#endif

#include "Camera.hpp"
#include "Color.hpp"
#include "GLFW/glfw3.h"
#include "Handle.hpp"
#include "Model.hpp"
#include "PbrMaterial.hpp"
#include "Query.hpp"
#include "Scheduler.hpp"
#include "pipeline.hpp"

template <typename Renderer>
class glWindow : public cevy::engine::Window::generic_window {
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
  using glLight = cevy::engine::pipeline::Light;

  public:
  glWindow(int width, int height) : width(width), height(height) {
    // std::cout << " <<<< glWindow(width, height) @" << this << "  <<<<" << std::endl;
    this->renderer = std::make_unique<Renderer>(*this);
    open();
    this->renderer->init();
  }
  glWindow(glWindow &&rhs) noexcept :  width(width), height(height), renderer(nullptr) {
    this->renderer.swap(rhs.renderer);
    // std::cout << " <<<< glWindow MOVE CONSTRUCT @" << this << "  <<<<" << std::endl;
    this->width = rhs.width;
    this->height = rhs.height;
    this->glfWindow = rhs.glfWindow;
    rhs.glfWindow = nullptr;

    glfwSetWindowUserPointer(this->glfWindow, this);
  }
  glWindow(const glWindow &) = delete;

  ~glWindow() {
    // std::cout << " <<<< ~glWindow @" << this << "  <<<<" << std::endl;

    this->renderer.reset();

    if (this->glfWindow) {
      /*importantly, since children depend on the gl context for destruction,
       we only destroy the gl context after destroying its dependants */
      std::cerr << " <<<< TERMINATING GL WINDOW <<<<" << std::endl;
      glfwDestroyWindow(this->glfWindow);
      glfwTerminate();
    }
  };

  glm::vec<2, int> size() const override {
    return {width, height};
  }
  void setSize(int /* width */, int /* height */) override {

  }
  void setFullscreen(bool /* fullscreen */) override {

  }

  bool open() override {
    this->init_context();
    return 0;
  }
  static void render_system(
      Resource<cevy::engine::Window> win,
      cevy::ecs::EventWriter<cevy::ecs::AppExit> close,
      cevy::ecs::World &world) {
    win.get().get_handler<glWindow, Renderer>()->render(close, world);
  }

  void
  render(
         cevy::ecs::EventWriter<cevy::ecs::AppExit> close,
         cevy::ecs::World &world) {

    glfwPollEvents();

    if (glfwWindowShouldClose(this->glfWindow)) {
      close.send(cevy::ecs::AppExit());
      return;
    }

    world.run_system_with(Renderer::render_system, *this->renderer);

    glfwSwapBuffers(this->glfWindow);
  }

  protected:
  void updateSize(int width, int height) {
    this->width = width;
    this->height = height;
  }
  void keyInput(int key, int /* scancode */, int action, int /* mods */) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      glfwSetWindowShouldClose(this->glfWindow, GLFW_TRUE);
  }
  void cursor(double /* xpos */, double /* ypos */) {}
  void mouseInput(int /* button */, int /* action */, int /* mods */) {}
  bool init_context() {
    if (!glfwInit()) {
      throw std::runtime_error("failed to init glfw");
      // Initialization failed
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    this->glfWindow = glfwCreateWindow(width, height, "C++evy glWindow", NULL, NULL);
    if (!this->glfWindow) {
      glfwTerminate();
      throw std::runtime_error("failed to create window");
    }
    glfwSetWindowUserPointer(this->glfWindow, this);
    glfwMakeContextCurrent(this->glfWindow);

    glfwSetWindowSizeCallback(this->glfWindow, [](GLFWwindow *win, int width, int height) {
      getFromWin(win)->updateSize(width, height);
    });
    glfwSetMouseButtonCallback(this->glfWindow,
                               [](GLFWwindow *win, int button, int action, int mods) {
                                 getFromWin(win)->mouseInput(button, action, mods);
                               });
    glfwSetCursorPosCallback(this->glfWindow, [](GLFWwindow *win, double xpos, double ypos) {
      getFromWin(win)->cursor(xpos, ypos);
    });
    glfwSetKeyCallback(this->glfWindow,
                       [](GLFWwindow *win, int key, int scancode, int action, int mods) {
                         getFromWin(win)->keyInput(key, scancode, action, mods);
                       });

#if _WIN32
    if (gl3wInit()) {
      fprintf(stderr, "failed to initialize OpenGL\n");
      return -1;
    }
    if (!gl3wIsSupported(4, 2)) {
      fprintf(stderr, "OpenGL 4.2 not supported\n");
      return -1;
    }
#elif __linux__
    if (glewInit()) {
      // std::string err(glewGetErrorString());
      throw std::runtime_error("failed to init glew");

      // fprintf(stderr, "failed to initialize OpenGL\n");
      return -1;
    }
    if (!glewIsSupported("GL_VERSION_4_2")) {
      throw std::runtime_error("glew unsupported");
      // fprintf(stderr, "OpenGL 4.2 not supported\n");
      return -1;
    }
#endif // _WIN32

    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION),
           glGetString(GL_SHADING_LANGUAGE_VERSION));
    glfwSwapInterval(1); // enable vsync
    return 0;
  }
  bool unload_context() {
    glfwTerminate();
    return 0;
  }

  static glWindow *getFromWin(GLFWwindow *glfWindow) {
    return static_cast<glWindow *>(glfwGetWindowUserPointer(glfWindow));
  }

  int width;
  int height;
  GLFWwindow *glfWindow;
  std::unique_ptr<Renderer> renderer;
};
