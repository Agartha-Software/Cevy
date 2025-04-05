/*
** Agartha-Software, 2024
** C++evy
** File description:
** openGL window handling
*/

// clang-format off
#include "Event.hpp"
// clang-format on
#include "Window.hpp"
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include "glx.hpp"
#include "cursor.hpp"
#include "Scheduler.hpp"
#include "input/state.hpp"
#include "glWindow.hpp"

glWindow::glWindow(int width, int height) : cevy::engine::Window::GenericWindow(width, height, false), targetSize(width, height) {
  this->init_context();
}

glWindow::glWindow(glWindow &&rhs) noexcept : cevy::engine::Window::GenericWindow(rhs.windowSize.x, rhs.windowSize.y, rhs.fullscreen) {
  this->modules = std::move(rhs.modules);
  this->module_keys = std::move(rhs.module_keys);
  rhs.modules.clear();
  rhs.module_keys.clear();
  this->windowSize = rhs.windowSize;
  this->renderSize = rhs.renderSize;
  this->targetSize = rhs.targetSize;
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

bool glWindow::isFullscreen() const {
  return fullscreen;
}

void glWindow::setFullscreen(bool fullscreen) {
  if (fullscreen && !this->fullscreen) {
    auto monitor = glfwGetPrimaryMonitor();
    auto mode = glfwGetVideoMode(monitor);

    glfwSetWindowMonitor(glfWindow, monitor, 0, 0, mode->width, mode->height , 60);
    this->targetSize = { mode->width, mode->height };
    this->windowSize = { mode->width, mode->height };
    this->fullscreen = true;
  } else if (!fullscreen && this->fullscreen) {
    glfwSetWindowMonitor(glfWindow, NULL, 0, 0, windowSize.x, windowSize.y, 60);
    this->fullscreen = false;
  }
}

void glWindow::setCursorState(cevy::engine::CursorState state) {
  glfwSetInputMode(glfWindow, GLFW_CURSOR, state);
}

void glWindow::init_system(Resource<cevy::engine::Window> win,
                        Resource<cevy::input::cursorInWindow> cursorInWindow,
                        EventWriter<cevy::input::keyboardInput> keyboardInputWriter,
                        EventWriter<cevy::input::mouseInput> mouseInputWriter,
                        EventWriter<cevy::input::cursorMoved> cursorMovedWriter,
                        EventWriter<cevy::input::windowFocused> windowFocusedWriter,
                        EventWriter<cevy::input::cursorEntered> cursor_entered_writer,
                        EventWriter<cevy::input::cursorLeft> cursor_left_writer) {
  glWindow &self = win->get_handler<glWindow>();

  cursorInWindow->inside = glfwGetWindowAttrib(self.glfWindow, GLFW_HOVERED);

  self.keyboardInputWriter.emplace(keyboardInputWriter);
  self.mouseInputWriter.emplace(mouseInputWriter);
  self.cursorMovedWriter.emplace(cursorMovedWriter);
  self.windowFocusedWriter.emplace(windowFocusedWriter);
  self.cursor_entered_writer.emplace(cursor_entered_writer);
  self.cursor_left_writer.emplace(cursor_left_writer);
  if (cursorInWindow->inside) {
    cursor_entered_writer.send(cevy::input::cursorEntered{});
  }
}

void glWindow::pre_render_system(Resource<cevy::engine::Window> win,
                              EventWriter<cevy::ecs::AppExit> close) {
  win.get().get_handler<glWindow>().preRender(close);
}

void glWindow::post_render_system(Resource<cevy::engine::Window> win) {
  win.get().get_handler<glWindow>().postRender();
}

void glWindow::preRender(EventWriter<cevy::ecs::AppExit> close) {
  if (glfwWindowShouldClose(this->glfWindow)) {
    close.send(cevy::ecs::AppExit());
    return;
  }
}

void glWindow::postRender() {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, this->framebuffer);

  glBlitFramebuffer(0, 0, this->windowSize.x, this->windowSize.y, 0, 0, this->windowSize.x, this->windowSize.y,
                  GL_COLOR_BUFFER_BIT, GL_NEAREST);

  glfwSwapBuffers(this->glfWindow);

  this->keyboardInputWriter->clear();
  this->mouseInputWriter->clear();
  this->cursorMovedWriter->clear();
  this->windowFocusedWriter->clear();
  this->cursor_entered_writer->clear();
  this->cursor_left_writer->clear();

  glfwPollEvents();
}

void glWindow::setWindowSize(int width, int height) {
  this->windowSize = { width, height };
  this->setTargetSize(width, height);
}

void glWindow::setTargetSize(int width, int height) {
  this->targetSize = { width, height };
  glBindTexture(GL_TEXTURE_2D, this->render_target);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, std::max(this->windowSize.x, width), std::max(this->windowSize.y, height), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void glWindow::setRenderSize(int width, int height) {
  this->renderSize = { width, height };
}

void glWindow::keyInput(int key, int /*scancode*/, int action, int /* mods */) {
  if (!this->keyboardInputWriter.has_value()) {
    throw std::runtime_error("callback access outside of poll");
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
  if (!this->cursor_entered_writer.has_value() || !this->cursor_left_writer.has_value()) {
    throw std::runtime_error("callback access outside of poll");
  }

  if (entered) {
    this->cursor_entered_writer->send(cevy::input::cursorEntered{});
  } else {
    this->cursor_left_writer->send(cevy::input::cursorLeft{});
  }
}

bool glWindow::init_context() {
// #if GLFW_HINT_X11
  glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
// #endif // GLFW_HINT_X11
  if (!glfwInit()) {
    throw std::runtime_error("failed to init glfw");
    // Initialization failed
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  this->glfWindow = glfwCreateWindow(this->windowSize.x, this->windowSize.y, "C++evy glWindow", NULL, NULL);
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
#endif // _WIN32 | __linux__

  glEnable(GL_DEBUG_OUTPUT);
  // typedef void APIENTRY _DEBUGPROC(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

  auto debug_func = [](GLenum source, GLenum type, GLuint /* id */, GLenum /* severity */, GLsizei /* length */, const GLchar* message, const void* /* userParam */) -> void {
    std::cerr << "DBG::" << source << "::{" << type << "}" << std::endl << std::string(message) << std::endl << std::endl;
    // std::cerr << "DBG::" << "::{" << type << "}" << std::string(message) << std::endl;
  };

  glDebugMessageCallback(debug_func, nullptr);

  printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION),
          glGetString(GL_SHADING_LANGUAGE_VERSION));
  glfwSwapInterval(0); // vsync disable

  glGenFramebuffers(1, &this->framebuffer);

  glGenTextures(1, &this->render_target);

  glBindTexture(GL_TEXTURE_2D, this->render_target);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->windowSize.x, this->windowSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
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
