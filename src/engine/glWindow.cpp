/*
** Agartha-Software, 2024
** C++evy
** File description:
** openGL window handling
*/

// clang-format off

#include "Event.hpp"
#include "ShaderProgram.hpp"
// clang-format on
#include "Window.hpp"
#include <GL/glext.h>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#if (_WIN32)
#include <GL/gl3w.h>
#endif
#if (__linux__)
#include <GL/glew.h>
#endif

#include "GLFW/glfw3.h"
#include "Scheduler.hpp"
#include "input/state.hpp"
#include "glWindow.hpp"

// glWindow::glWindow(int width, int height) : window_size(width, height), render_size(width, height) {
glWindow::glWindow(int width, int height) : window_size(width, height), render_size(width * 2 / 3, height * 2 / 3), target_size(width, height) {
  open();
  // this->renderer = std::make_unique<Renderer>(*this);
}

glWindow::glWindow(glWindow &&rhs) noexcept {
  this->modules = std::move(rhs.modules);
  this->module_keys = std::move(rhs.module_keys);
  rhs.modules.clear();
  rhs.module_keys.clear();
  this->window_size = rhs.window_size;
  this->render_size = rhs.render_size;
  this->target_size = rhs.target_size;
  this->glfWindow = rhs.glfWindow;
  this->framebuffer = rhs.framebuffer;
  rhs.framebuffer = 0;
  this->render_target = rhs.render_target;
  rhs.render_target = 0;
  rhs.glfWindow = nullptr;

  glfwSetWindowUserPointer(this->glfWindow, this);
}

glWindow::~glWindow() {
  // this->renderer.reset();

  for (auto &module : this->modules) {
    module->deinit(*this);
  }
  this->modules.clear();
  this->module_keys.clear();

  if (this->glfWindow) {
    /*importantly, since children depend on the gl context for destruction,
      we only destroy the gl context after destroying its dependants */
    std::cerr << " <<<< TERMINATING GL WINDOW <<<<" << std::endl;

    glDeleteFramebuffers(1, &this->framebuffer);
    glDeleteTextures(1, &this->render_target);
    this->render_target = 0;

    glfwDestroyWindow(this->glfWindow);
    glfwTerminate();
  }
};

glm::vec<2, int> glWindow::windowSize() const { return window_size; }
glm::vec<2, int> glWindow::renderSize() const { return render_size; }
glm::vec<2, int> glWindow::targetSize() const { return target_size; }

void glWindow::setFullscreen(bool /* fullscreen */) {}

bool glWindow::open() {
  this->init_context();
  return 0;
}

void glWindow::init_system(Resource<cevy::engine::Window> win,
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

void glWindow::pre_render_system(Resource<cevy::engine::Window> win,
                              EventWriter<cevy::ecs::AppExit> close) {
  win.get().get_handler<glWindow>().pre_render(close);
}

void glWindow::post_render_system(Resource<cevy::engine::Window> win) {
  win.get().get_handler<glWindow>().post_render();
}

void glWindow::pre_render(EventWriter<cevy::ecs::AppExit> close) {
  if (glfwWindowShouldClose(this->glfWindow)) {
    close.send(cevy::ecs::AppExit());
    return;
  }
}

void glWindow::post_render() {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, this->framebuffer);

  // std::cout << this->final_size.x << "  " << this->final_size.y << std::endl;
  glBlitFramebuffer(0, 0, this->window_size.x, this->window_size.y, 0, 0, this->window_size.x, this->window_size.y,
                  GL_COLOR_BUFFER_BIT, GL_NEAREST);

  glfwSwapBuffers(this->glfWindow);

  this->keyboardInputWriter->clear();
  this->mouseInputWriter->clear();
  this->cursorMovedWriter->clear();
  this->windowFocusedWriter->clear();
  this->cursorEnteredWriter->clear();
  this->cursorLeftWriter->clear();

  glfwPollEvents();
}

void glWindow::pollEvents() {
  glfwPollEvents();
}

void glWindow::setWindowSize(int width, int height) {
  this->window_size = { width, height };
  glBindTexture(GL_TEXTURE_2D, this->render_target);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, std::max(this->target_size.x, width), std::max(this->target_size.y, height), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void glWindow::setTargetSize(int width, int height) {
  this->target_size = { width, height };
  glBindTexture(GL_TEXTURE_2D, this->render_target);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, std::max(this->window_size.x, width), std::max(this->window_size.y, height), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void glWindow::setRenderSize(int width, int height) {
  this->render_size = { width, height };
}

void glWindow::keyInput(int key, int /*scancode*/, int action, int /* mods */) {
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

void glWindow::cursor(double xpos, double ypos) {
  if (!this->cursorMovedWriter.has_value()) {
    throw std::runtime_error("callback access outside of poll");
  }
  this->cursorMovedWriter->send(cevy::input::cursorMoved{{xpos, ypos}});
}

void glWindow::windowFocused(int focused) {
  if (!this->windowFocusedWriter.has_value()) {
    throw std::runtime_error("callback access outside of poll");
  }
  this->windowFocusedWriter->send(cevy::input::windowFocused{bool(focused)});
}

void glWindow::mouseInput(int button, int action, int /* mods */) {
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

void glWindow::cursorEnter(int entered) {
  if (!this->cursorEnteredWriter.has_value() || !this->cursorLeftWriter.has_value()) {
    throw std::runtime_error("callback access outside of poll");
  }

  if (entered) {
    this->cursorEnteredWriter->send(cevy::input::cursorEntered{});
  } else {
    this->cursorLeftWriter->send(cevy::input::cursorLeft{});
  }
}

bool glWindow::init_context() {
  if (!glfwInit()) {
    throw std::runtime_error("failed to init glfw");
    // Initialization failed
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  this->glfWindow = glfwCreateWindow(window_size.x, window_size.y, "C++evy glWindow", NULL, NULL);
  if (!this->glfWindow) {
    glfwTerminate();
    throw std::runtime_error("failed to create window");
  }
  glfwSetWindowUserPointer(this->glfWindow, this);
  glfwMakeContextCurrent(this->glfWindow);

  glfwSetWindowSizeCallback(this->glfWindow, [](GLFWwindow *win, int width, int height) {
    getFromWin(win)->setWindowSize(width, height);
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

  glGenFramebuffers(1, &this->framebuffer);

  glGenTextures(1, &this->render_target);

  glBindTexture(GL_TEXTURE_2D, this->render_target);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->window_size.x, this->window_size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->render_target, 0);

  return 0;
}

bool glWindow::unload_context() {
  glfwTerminate();
  return 0;
}

glWindow *glWindow::getFromWin(GLFWwindow *glfWindow) {
  return static_cast<glWindow *>(glfwGetWindowUserPointer(glfWindow));
}

GLFWwindow *glWindow::getGLFWwindow() const {
  return glfWindow;
}
