/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Editor
*/

#include "Editor.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void cevy::editor::Editor::init(glWindow &glwindow) {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(glwindow.getGLFWwindow(),
                                true); // Second param install_callback=true will install GLFW
                                    // callbacks and chain to existing ones.
  ImGui_ImplOpenGL3_Init();
}

void cevy::editor::Editor::deinit(glWindow &) {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void cevy::editor::Editor::build(cevy::ecs::App &app) {
  app.add_stage<EditorPreRender>();
  app.add_stage<EditorRender>();
  app.add_systems<EditorPreRender>(Editor::pre_render);
  app.add_systems<EditorRender>(Editor::render);
}

void cevy::editor::Editor::pre_render() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGui::ShowDemoWindow(); // Show demo window! :)
}

void cevy::editor::Editor::render() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
