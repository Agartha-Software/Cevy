/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Editor
*/

#include "Editor.hpp"

#include "Event.hpp"
#include "Window.hpp"
#include "glWindow.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "input/state.hpp"

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
  // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

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

void cevy::editor::Editor::build(cevy::ecs::App &app) {
  app.add_stage<EditorPreRender>();
  app.add_stage<EditorRender>();
  app.add_stage<EditorInput>();
  app.add_systems<EditorPreRender>(Editor::pre_render);
  app.add_systems<EditorRender>(Editor::render);
  app.add_systems<EditorInput>(Editor::intercept_inputs);
}

void cevy::editor::Editor::intercept_inputs(cevy::ecs::World &world,
  cevy::ecs::EventWriter<cevy::input::cursorMoved> cursorMoved,
  cevy::ecs::Resource<cevy::engine::Window> windower) {
  auto &glwindow = windower->get_handler<glWindow>();
  auto &self = glwindow.get_module<Editor>();
  auto o_cursor_moved = world.get_resource<ecs::Event<cevy::input::cursorMoved>>();

  if (o_cursor_moved.has_value()) {
    o_cursor_moved->get().event_queue.clear();
  }

  if (self.viewport_pos.has_value() && self.viewport_size.has_value()) {
    auto io = ImGui::GetIO();
    ImVec2 screen_pos = io.MousePos;
    int x_pos = screen_pos.x - self.viewport_pos->x;
    int y_pos = screen_pos.y - self.viewport_pos->y;
    if (x_pos < 0 || x_pos > self.viewport_size->x || y_pos < 0 || y_pos > self.viewport_size->y) {
      return;
    }
    cursorMoved.send(cevy::input::cursorMoved { { screen_pos.x - self.viewport_pos->x, screen_pos.y - self.viewport_pos->y } });
  }
}

void cevy::editor::Editor::pre_render(cevy::ecs::Resource<cevy::engine::Window> windower) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  auto &glwindow = windower->get_handler<glWindow>();
  auto &self = glwindow.get_module<Editor>();
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
    self.viewport_pos = ImGui::GetWindowPos();
    self.viewport_size = ImGui::GetWindowSize();
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

void cevy::editor::Editor::render(cevy::ecs::Resource<cevy::engine::Window> windower) {
  auto &glwindow = windower->get_handler<glWindow>();

  auto &self = glwindow.get_module<Editor>();

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
