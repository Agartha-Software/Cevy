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
// clang-format on
#include "Window.hpp"
#include "cevy.hpp"
#include <glm/fwd.hpp>
#include <optional>
#include "glx.hpp"

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
    //virtual Module(glWindow&) = 0;
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
  glWindow(int width, int height);

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

  glWindow(glWindow &&rhs) noexcept;
  glWindow(const glWindow &) = delete;

  ~glWindow();

  glm::vec<2, int> windowSize() const override;
  glm::vec<2, int> renderSize() const override;
  glm::vec<2, int> targetSize() const;

  void setFullscreen(bool /* fullscreen */) override;

  bool open() override;

  static void init_system(Resource<cevy::engine::Window> win,
                          Resource<cevy::input::cursorInWindow> cursorInWindow,
                          EventWriter<cevy::input::keyboardInput> keyboardInputWriter,
                          EventWriter<cevy::input::mouseInput> mouseInputWriter,
                          EventWriter<cevy::input::cursorMoved> cursorMovedWriter,
                          EventWriter<cevy::input::windowFocused> windowFocusedWriter,
                          EventWriter<cevy::input::cursorEntered> cursorEnteredWriter,
                          EventWriter<cevy::input::cursorLeft> cursorLeftWriter);

  static void pre_render_system(Resource<cevy::engine::Window> win,
                            EventWriter<cevy::ecs::AppExit> close);

  static void post_render_system(Resource<cevy::engine::Window> win);

  void pre_render(EventWriter<cevy::ecs::AppExit> close);

  void post_render();

  void pollEvents() override;

  std::optional<EventWriter<cevy::input::keyboardInput>> keyboardInputWriter;
  std::optional<EventWriter<cevy::input::mouseInput>> mouseInputWriter;

  std::optional<EventWriter<cevy::input::cursorMoved>> cursorMovedWriter;
  std::optional<EventWriter<cevy::input::cursorEntered>> cursorEnteredWriter;
  std::optional<EventWriter<cevy::input::cursorLeft>> cursorLeftWriter;

  std::optional<EventWriter<cevy::input::windowFocused>> windowFocusedWriter;

  void setWindowSize(int width, int height) override;
  void setRenderSize(int width, int height) override;
  void setTargetSize(int width, int height);
  protected:
  void keyInput(int key, int /*scancode*/, int action, int /* mods */);

  void cursor(double xpos, double ypos);

  void windowFocused(int focused);

  void mouseInput(int button, int action, int /* mods */);

  void cursorEnter(int entered);

  bool init_context();

  bool unload_context();


  public:
  static glWindow *getFromWin(GLFWwindow *glfWindow);
  GLFWwindow *getGLFWwindow() const;
  GLuint getCurrentFrameBuffer() {
    return framebuffer;
  }

  GLuint getRenderTarget() const {
    return this->render_target;
  }

  protected:
  glm::vec<2, int>  window_size;
  glm::vec<2, int>  render_size;
  glm::vec<2, int>  target_size;
  GLFWwindow *glfWindow;
  GLuint framebuffer;
  GLuint render_target;
  PbrMaterial defaultMaterial;
  std::vector<std::unique_ptr<Module>> modules;
  std::unordered_map<std::type_index, size_t> module_keys;
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
