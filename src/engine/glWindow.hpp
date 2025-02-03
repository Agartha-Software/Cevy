/*
** Agartha-Software, 2024
** C++evy
** File description:
** openGL window handling
*/

#pragma once

// clang-format off
#include "App.hpp"
#include "Event.hpp"
#include "Plugin.hpp"
#include "ShaderProgram.hpp"
// clang-format on
#include "Window.hpp"
#include <optional>
#include <stdexcept>
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
#include "input/state.hpp"

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
  class Plugin : public cevy::ecs::Plugin {
    public:
    void build(cevy::ecs::App &app) override {
      app.add_systems<cevy::ecs::core_stage::Startup>(glWindow<Renderer>::init_system);
      app.add_systems<cevy::engine::RenderStage>(glWindow<Renderer>::render_system);
    }
  };

  glWindow(int width, int height) : width(width), height(height) {
    this->renderer = std::make_unique<Renderer>(*this);
    open();
    this->renderer->init();
  }
  glWindow(glWindow &&rhs) noexcept :  width(width), height(height), renderer(nullptr) {
    this->renderer.swap(rhs.renderer);
    this->width = rhs.width;
    this->height = rhs.height;
    this->glfWindow = rhs.glfWindow;
    rhs.glfWindow = nullptr;

    glfwSetWindowUserPointer(this->glfWindow, this);
  }
  glWindow(const glWindow &) = delete;

  ~glWindow() {
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

  static void init_system(
      Resource<cevy::engine::Window> win,
      cevy::ecs::EventWriter<cevy::input::keyPressed> keyPressedWriter,
      cevy::ecs::EventWriter<cevy::input::keyReleased> keyReleasedWriter,
      cevy::ecs::EventWriter<cevy::input::cursorMoved> cursorMovedWriter,
      cevy::ecs::EventWriter<cevy::input::windowFocused> windowFocusedWriter
  ) {
    glWindow<Renderer>& self = *win->get_handler<glWindow, Renderer>();
    self.keyReleasedWriter.emplace(keyReleasedWriter);
    self.keyPressedWriter.emplace(keyPressedWriter);
    self.cursorMovedWriter.emplace(cursorMovedWriter);
    self.windowFocusedWriter.emplace(windowFocusedWriter);
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
    if (glfwWindowShouldClose(this->glfWindow)) {
      close.send(cevy::ecs::AppExit());
      return;
    }

    world.run_system_with(Renderer::render_system, *this->renderer);

    glfwSwapBuffers(this->glfWindow);

    this->keyReleasedWriter->clear();
    this->keyPressedWriter->clear();
    this->cursorMovedWriter->clear();
    this->windowFocusedWriter->clear();

    glfwPollEvents();
  }

  void pollEvents() {
    glfwPollEvents();
  }

  std::optional<cevy::ecs::EventWriter<cevy::input::keyPressed>> keyPressedWriter;
  std::optional<cevy::ecs::EventWriter<cevy::input::keyReleased>> keyReleasedWriter;

  std::optional<cevy::ecs::EventWriter<cevy::input::cursorMoved>> cursorMovedWriter;

  std::optional<cevy::ecs::EventWriter<cevy::input::windowFocused>> windowFocusedWriter;

  protected:
  void updateSize(int width, int height) {
    this->width = width;
    this->height = height;
  }

  void keyInput(int key, int /*scancode*/, int action, int /* mods */) {
    if (!this->keyPressedWriter.has_value() || !this->keyReleasedWriter.has_value()) {
      throw std::runtime_error("callback access outside of poll");
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      glfwSetWindowShouldClose(glfWindow, GLFW_TRUE);
    if (action == GLFW_PRESS)
      this->keyPressedWriter.value().send(cevy::input::keyPressed { static_cast<cevy::input::KeyCode>(key) });
    if (action == GLFW_RELEASE)
      this->keyReleasedWriter.value().send(cevy::input::keyReleased { static_cast<cevy::input::KeyCode>(key) });
  }


  void cursor(double xpos, double ypos) {
    if (!this->cursorMovedWriter.has_value()) {
      throw std::runtime_error("callback access outside of poll");
    }
    this->cursorMovedWriter.value().send(cevy::input::cursorMoved { { xpos, ypos }} );
  }

  void windowFocused(int focused) {
    if (!this->windowFocusedWriter.has_value()) {
      throw std::runtime_error("callback access outside of poll");
    }
    this->windowFocusedWriter.value().send(cevy::input::windowFocused { bool(focused)});
  }

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
    glfwSetWindowFocusCallback(this->glfWindow,
                                [](GLFWwindow* win, int focused) {
                         getFromWin(win)->windowFocused(focused);
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
  PbrMaterial defaultMaterial;
  std::unique_ptr<Renderer> renderer;
};
