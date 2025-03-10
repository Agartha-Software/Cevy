/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Editor Game Windows
*/

#include "GameWindow.hpp"
#include "Editor.hpp"
#include "imgui.h"

void cevy::editor::GameWindow::render(cevy::editor::Editor &editor, glWindow &glwindow) {
  ImGui::Begin("GameWindow");
  // Using a Child allow to fill all the space of the window.
  // It also alows customization
  ImGui::BeginChild("GameRender");
  // Get the size of the child (i.e. the whole draw size of the windows).
  editor.viewportPos = ImGui::GetWindowPos();
  editor.viewportSize = ImGui::GetWindowSize();
  ImVec2 wsize = ImGui::GetWindowSize();
  if (wsize.x != glwindow.targetSize().x || wsize.y != glwindow.targetSize().y) {
    glwindow.setTargetSize(wsize.x, wsize.y);
    glBindTexture(GL_TEXTURE_2D, editor.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, wsize.x, wsize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  }
  // Because I use the texture from OpenGL, I need to invert the V from the UV.
  ImGui::Image((ImTextureID)editor.texture, wsize, ImVec2(0, 1), ImVec2(1, 0));
  ImGui::EndChild();
  ImGui::End();
}
