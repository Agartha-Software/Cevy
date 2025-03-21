/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Editor Profiling Windows
*/

#include "ProfilingWindow.hpp"
#include "Editor.hpp"
#include "LegitProfiler/ProfilerTask.hpp"
#include "imgui.h"
#include <chrono>

void cevy::editor::ProfilingWindow::render(cevy::editor::Editor &, glWindow &) {
  auto now = std::chrono::high_resolution_clock::now();
  auto elapsed_time = now - this->last_call;
  this->frames.push_back(
      1. /
      (std::chrono::duration_cast<std::chrono::microseconds>(elapsed_time).count() / 1000000.f));
  if (this->frames.size() > 120) {
    this->frames.erase(frames.begin());
  }
  this->framesAverage.push_back(ImGui::GetIO().Framerate);
  if (this->framesAverage.size() > 120) {
    this->framesAverage.erase(framesAverage.begin());
  }
  ImGui::Text("Elapsed time since last frame: %.3f ms",
              std::chrono::duration_cast<std::chrono::microseconds>(elapsed_time).count() / 1000.f);
  ImGui::Text("Current FPS %.3f", ImGui::GetIO().Framerate);
  ImGui::Text("Framerate");
  ImGui::PlotHistogram("##Framerate", &frames[0], frames.size(), 0, NULL, 0.0f, 140.0f,
                       ImVec2(300, 100));
  ImGui::Text("Running average framerate");
  ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(0, 235, 90, 255));
  ImGui::PlotHistogram("##Running average framerate", &framesAverage[0], frames.size(), 0, NULL,
                       0.0f, 140.0f, ImVec2(300, 100));
  ImGui::PopStyleColor();

  std::vector<legit::ProfilerTask> tasks(
      {legit::ProfilerTask {0, (double) std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time).count() / 2, std::string("Bite"), legit::Colors::alizarin},
    legit::ProfilerTask {(double) std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time).count() / 2, (double) std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time).count(), std::string("Bate"), legit::Colors::amethyst}});

  legitProfiler.cpuGraph.LoadFrameData(tasks.data(), 2);
  legitProfiler.gpuGraph.LoadFrameData(tasks.data(), 2);
  legitProfiler.Render();

  this->last_call = now;
}
