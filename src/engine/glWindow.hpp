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
#include "cevy.hpp"
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
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
#include "input/state.hpp"
#include "pipeline.hpp"

class glWindow : public cevy::engine::Window::generic_window {
  public:

  template<typename... Modules>
  struct Builder ;

  struct Module {
    virtual void init(glWindow&) = 0;
    virtual void deinit(glWindow&) = 0;
    virtual void build(cevy::ecs::App &app) = 0;
  };

  protected:
  template <typename T>
  using Handle = cevy::engine::Handle<T>;

  template <typename T>
  using Resource = cevy::ecs::Resource<T>;
  template <typename... T>
  using Query = cevy::ecs::Query<T...>;
  template <typename T>
  using EventWriter = cevy::ecs::EventWriter<T>;

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
      app.add_systems<cevy::ecs::core_stage::Startup>(glWindow::init_system);
      app.add_systems<cevy::engine::PreRenderStage>(glWindow::pre_render_system);
      app.add_systems<cevy::engine::PostRenderStage>(glWindow::post_render_system);
    }
  };

  protected:
  glWindow(int width, int height) : width(width), height(height) {
    open();

    // this->renderer = std::make_unique<Renderer>(*this);
  }
  public:

  template<typename... Mod>
  glWindow& add_modules() {
    static_assert(all(std::is_base_of_v<Module, Mod>...),
            "Given Modules do not derive from Module class");
    ([this](){
      this->module_keys.emplace(std::type_index(typeid(Mod)), this->modules.size());
      this->modules.push_back(std::make_unique<Mod>(*this));
      this->modules.back()->init(*this);
    }(), ...);
    return *this;
  }

  template<typename Mod>
  glWindow& add_module() {
    static_assert(std::is_base_of_v<Module, Mod>,
            "Given Module does not derive from Module class");
    this->module_keys.emplace(std::type_index(typeid(Mod)), this->modules.size());
    this->modules.push_back(std::make_unique<Mod>(*this));
    this->modules.back()->init(*this);
    return *this;
  }

  template<typename Mod>
  Mod& get_module() {
    auto key = this->module_keys.at(std::type_index(typeid(Mod)));
    return dynamic_cast<Mod&>(*this->modules[key]);
  }

  glWindow(glWindow &&rhs) noexcept {
    this->modules = std::move(rhs.modules);
    this->module_keys = std::move(rhs.module_keys);
    rhs.modules.clear();
    rhs.module_keys.clear();
    this->width = rhs.width;
    this->height = rhs.height;
    this->glfWindow = rhs.glfWindow;
    rhs.glfWindow = nullptr;

    glfwSetWindowUserPointer(this->glfWindow, this);
  }
  glWindow(const glWindow &) = delete;

  ~glWindow() {
    // this->renderer.reset();

    for (auto& module: this->modules) {
      module->deinit(*this);
    }
    this->modules.clear();
    this->module_keys.clear();


    if (this->glfWindow) {
      // ImGui_ImplOpenGL3_Shutdown();
      // ImGui_ImplGlfw_Shutdown();
      // ImGui::DestroyContext();
      /*importantly, since children depend on the gl context for destruction,
       we only destroy the gl context after destroying its dependants */
      std::cerr << " <<<< TERMINATING GL WINDOW <<<<" << std::endl;
      glfwDestroyWindow(this->glfWindow);
      glfwTerminate();
    }
  };

  glm::vec<2, int> size() const override { return {width, height}; }

  void setSize(int /* width */, int /* height */) override {}
  void setFullscreen(bool /* fullscreen */) override {}

  bool open() override {
    this->init_context();
    return 0;
  }

  static void init_system(Resource<cevy::engine::Window> win,
                          Resource<cevy::input::cursorInWindow> cursorInWindow,
                          EventWriter<cevy::input::keyboardInput> keyboardInputWriter,
                          EventWriter<cevy::input::mouseInput> mouseInputWriter,
                          EventWriter<cevy::input::cursorMoved> cursorMovedWriter,
                          EventWriter<cevy::input::windowFocused> windowFocusedWriter,
                          EventWriter<cevy::input::cursorEntered> cursorEnteredWriter,
                          EventWriter<cevy::input::cursorLeft> cursorLeftWriter) {
    glWindow &self = win->get_handler<glWindow>();

    cursorInWindow->inside = glfwGetWindowAttrib(self.glfWindow, GLFW_HOVERED);

    self.keyboardInputWriter.emplace(keyboardInputWriter);
    self.mouseInputWriter.emplace(mouseInputWriter);
    self.cursorMovedWriter.emplace(cursorMovedWriter);
    self.windowFocusedWriter.emplace(windowFocusedWriter);
    self.cursorEnteredWriter.emplace(cursorEnteredWriter);
    self.cursorLeftWriter.emplace(cursorLeftWriter);
    if (cursorInWindow->inside) {
      cursorEnteredWriter.send(cevy::input::cursorEntered{});
    }
  }

  static void pre_render_system(Resource<cevy::engine::Window> win,
                            EventWriter<cevy::ecs::AppExit> close, cevy::ecs::World &world) {
    win.get().get_handler<glWindow>().pre_render(close, world);
  }

  static void post_render_system(Resource<cevy::engine::Window> win,
                            EventWriter<cevy::ecs::AppExit> close, cevy::ecs::World &world) {
    win.get().get_handler<glWindow>().post_render(close, world);
  }

  void pre_render(EventWriter<cevy::ecs::AppExit> close, cevy::ecs::World &world) {
    if (glfwWindowShouldClose(this->glfWindow)) {
      close.send(cevy::ecs::AppExit());
      return;
    }
  }

  void post_render(EventWriter<cevy::ecs::AppExit> close, cevy::ecs::World &world) {
    // ImGui_ImplOpenGL3_NewFrame();
    // ImGui_ImplGlfw_NewFrame();
    // ImGui::NewFrame();
    // ImGui::ShowDemoWindow(); // Show demo window! :)

    // (world.run_system_with(Mod::render_system, *this->renderer), ...);

    // ImGui::Render();
    // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(this->glfWindow);

    this->keyboardInputWriter->clear();
    this->mouseInputWriter->clear();
    this->cursorMovedWriter->clear();
    this->windowFocusedWriter->clear();
    this->cursorEnteredWriter->clear();
    this->cursorLeftWriter->clear();

    glfwPollEvents();

  }

  void pollEvents() override { glfwPollEvents(); }

  std::optional<EventWriter<cevy::input::keyboardInput>> keyboardInputWriter;
  std::optional<EventWriter<cevy::input::mouseInput>> mouseInputWriter;

  std::optional<EventWriter<cevy::input::cursorMoved>> cursorMovedWriter;
  std::optional<EventWriter<cevy::input::cursorEntered>> cursorEnteredWriter;
  std::optional<EventWriter<cevy::input::cursorLeft>> cursorLeftWriter;

  std::optional<EventWriter<cevy::input::windowFocused>> windowFocusedWriter;

  protected:
  void updateSize(int width, int height) {
    this->width = width;
    this->height = height;
  }

  void keyInput(int key, int /*scancode*/, int action, int /* mods */) {
    if (!this->keyboardInputWriter.has_value()) {
      throw std::runtime_error("callback access outside of poll");
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      glfwSetWindowShouldClose(glfWindow, GLFW_TRUE);
    }

    if (action == GLFW_PRESS) {
      this->keyboardInputWriter->send(
          cevy::input::keyboardInput{static_cast<cevy::input::KeyCode>(key), true});
    }

    if (action == GLFW_RELEASE) {
      this->keyboardInputWriter->send(
          cevy::input::keyboardInput{static_cast<cevy::input::KeyCode>(key), false});
    }
  }

  void cursor(double xpos, double ypos) {
    if (!this->cursorMovedWriter.has_value()) {
      throw std::runtime_error("callback access outside of poll");
    }
    this->cursorMovedWriter->send(cevy::input::cursorMoved{{xpos, ypos}});
  }

  void windowFocused(int focused) {
    if (!this->windowFocusedWriter.has_value()) {
      throw std::runtime_error("callback access outside of poll");
    }
    this->windowFocusedWriter->send(cevy::input::windowFocused{bool(focused)});
  }

  void mouseInput(int button, int action, int /* mods */) {
    if (!this->mouseInputWriter.has_value()) {
      throw std::runtime_error("callback access outside of poll");
    }

    if (action == GLFW_PRESS) {
      this->mouseInputWriter->send(
          cevy::input::mouseInput{static_cast<cevy::input::MouseButton>(button), true});
    }

    if (action == GLFW_RELEASE) {
      this->mouseInputWriter->send(
          cevy::input::mouseInput{static_cast<cevy::input::MouseButton>(button), false});
    }
  }

  void cursorEnter(int entered) {
    if (!this->cursorEnteredWriter.has_value() || !this->cursorLeftWriter.has_value()) {
      throw std::runtime_error("callback access outside of poll");
    }

    if (entered) {
      this->cursorEnteredWriter->send(cevy::input::cursorEntered{});
    } else {
      this->cursorLeftWriter->send(cevy::input::cursorLeft{});
    }
  }

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
    glfwSetWindowFocusCallback(this->glfWindow, [](GLFWwindow *win, int focused) {
      getFromWin(win)->windowFocused(focused);
    });
    glfwSetCursorEnterCallback(this->glfWindow, [](GLFWwindow *win, int entered) {
      getFromWin(win)->cursorEnter(entered);
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

    // Setup Dear ImGui context
    // IMGUI_CHECKVERSION();
    // ImGui::CreateContext();
    // ImGuiIO& io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    // //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

    // ImGui::StyleColorsDark();
    // // Setup Platform/Renderer backends
    // ImGui_ImplGlfw_InitForOpenGL(this->glfWindow, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    // ImGui_ImplOpenGL3_Init();
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
  std::vector<std::unique_ptr<Module>> modules;
  std::unordered_map<std::type_index, size_t> module_keys;
  // std::unique_ptr<Renderer> renderer;
};

template<typename... Mod>
struct glWindow::Builder : public glWindow {
  Builder(int width, int height) : glWindow(width, height) {
    this->add_modules<Mod...>();
  }
  struct Plugin : public cevy::ecs::Plugin {
    void build(cevy::ecs::App& app) override {
      app.add_plugins(glWindow::Plugin());
      for (auto& module : app.resource<cevy::engine::Window>().get_handler<glWindow>().modules) {
        module->build(app);
      }
      // app.add_plugins(typename Mod::Plugin()...);
    }
  };
};
