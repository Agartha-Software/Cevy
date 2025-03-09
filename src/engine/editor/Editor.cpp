/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Editor
*/

#include "Editor.hpp"

#include "Event.hpp"
#include "Resource.hpp"
#include "Window.hpp"
#include "engine.hpp"
#include "glWindow.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "state.hpp"

#include <GL/gl.h>

void cevy::editor::Editor::init(glWindow &glwindow) {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(glwindow.getGLFWwindow(),
                                true); // Second param install_callback=true will install GLFW
                                    // callbacks and chain to existing ones.
  ImGui_ImplOpenGL3_Init();

  glGenFramebuffers(1, &this->framebuffer);
  glGenTextures(1, &this->texture);
  glBindTexture(GL_TEXTURE_2D, this->texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, glwindow.windowSize().x, glwindow.windowSize().y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void cevy::editor::Editor::deinit(glWindow &) {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glDeleteTextures(1, &this->texture);
  glDeleteFramebuffers(1, &this->framebuffer);
}

void intercept_default_cursor_placement(cevy::ecs::Resource<cevy::input::cursorInWindow> inWindow) {
  inWindow->inside = false;
}

void clean_gl_window_inputs(cevy::ecs::World &world) {
  auto o_cursor_moved = world.get_resource<cevy::ecs::Event<cevy::input::cursorMoved>>();
  auto o_cursor_entered = world.get_resource<cevy::ecs::Event<cevy::input::cursorEntered>>();
  auto o_cursor_left = world.get_resource<cevy::ecs::Event<cevy::input::cursorLeft>>();

  if (o_cursor_moved.has_value()) {
    o_cursor_moved->get().event_queue.clear();
  }
  if (o_cursor_entered.has_value()) {
    o_cursor_entered->get().event_queue.clear();
  }
  if (o_cursor_left.has_value()) {
    o_cursor_left->get().event_queue.clear();
  }
}

void intercept_inputs(
  cevy::ecs::EventWriter<cevy::input::cursorMoved> cursor_moved,
  cevy::ecs::EventWriter<cevy::input::cursorEntered> cursor_entered_writer,
  cevy::ecs::EventWriter<cevy::input::cursorLeft> cursor_left_writer,
  cevy::ecs::Resource<cevy::engine::Window> windower) {
  auto &glwindow = windower->get_handler<glWindow>();
  auto &self = glwindow.get_module<cevy::editor::Editor>();

  if (!self.viewportPos.has_value() || !self.viewportSize.has_value()) {
    return;
  }

  ImVec2 screen_pos = ImGui::GetIO().MousePos;
  int x_pos = screen_pos.x - self.viewportPos->x;
  int y_pos = screen_pos.y - self.viewportPos->y;

  if (x_pos < 0 || x_pos > self.viewportSize->x || y_pos < 0 || y_pos > self.viewportSize->y) {
    if (self.cursorInViewport.has_value() && self.cursorInViewport.value()) {
      cursor_left_writer.send(cevy::input::cursorLeft {});
    }

    self.cursorInViewport = false;
  } else {
    cursor_moved.send(cevy::input::cursorMoved { { screen_pos.x - self.viewportPos->x, screen_pos.y - self.viewportPos->y } });

    // || !self.cursorInViewport.has_value() Is there to set the cursorInWindow to true as soon as possible if the cursor start in the viewport
    if ((self.cursorInViewport.has_value() && !self.cursorInViewport.value()) || !self.cursorInViewport.has_value()) {
      cursor_entered_writer.send(cevy::input::cursorEntered {});
    }

    self.cursorInViewport = true;
  }
}

void pre_render(cevy::ecs::Resource<cevy::engine::Window> windower) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  auto &glwindow = windower->get_handler<glWindow>();
  auto &self = glwindow.get_module<cevy::editor::Editor>();
  ImGui::ShowDemoWindow(); // Show demo window! :)
  auto io = ImGui::GetIO();
  //ImGui:: SetNextWindowSize(io.DisplaySize);
  //ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::Begin("GameWindow");
  {
    // Using a Child allow to fill all the space of the window.
    // It also alows customization
    ImGui::BeginChild("GameRender");
    // Get the size of the child (i.e. the whole draw size of the windows).
    self.viewportPos = ImGui::GetWindowPos();
    self.viewportSize = ImGui::GetWindowSize();
    ImVec2 wsize = ImGui::GetWindowSize();
    if (wsize.x != glwindow.targetSize().x || wsize.y != glwindow.targetSize().y) {
      glwindow.setTargetSize(wsize.x, wsize.y);
      glBindTexture(GL_TEXTURE_2D, self.texture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, wsize.x, wsize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    }
    // Because I use the texture from OpenGL, I need to invert the V from the UV.
    ImGui::Image((ImTextureID)self.texture, wsize, ImVec2(0, 1), ImVec2(1, 0));
    ImGui::EndChild();
  }
  ImGui::End();
}

void render(cevy::ecs::Resource<cevy::engine::Window> windower) {
  auto &glwindow = windower->get_handler<glWindow>();

  auto &self = glwindow.get_module<cevy::editor::Editor>();

  glBindFramebuffer(GL_READ_FRAMEBUFFER, glwindow.getCurrentFrameBuffer());
  glNamedFramebufferReadBuffer(glwindow.getCurrentFrameBuffer(), GL_BACK_LEFT);

  glViewport(0, 0, INT_MAX, INT_MAX);
  glBindTexture(GL_TEXTURE_2D, self.texture);
  glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, glwindow.targetSize().x, glwindow.targetSize().y, 0);

  ImGui::Render();
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  ImGuiIO &io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
      GLFWwindow* backup_current_context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
  }
}

void cevy::editor::Editor::build(cevy::ecs::App &app) {
  app.add_stage<EditorPreRender>();
  app.add_stage<EditorRender>();
  app.add_stage<EditorInput>();
  app.add_systems<engine::StartupRenderStage>(intercept_default_cursor_placement);
  app.add_systems<EditorPreRender>(pre_render);
  app.add_systems<EditorRender>(render);
  app.add_systems<EditorInput>(clean_gl_window_inputs);
  app.add_systems<EditorInput>(intercept_inputs);
}
