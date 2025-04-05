/*
** AgarthaSoftware, 2025
** Cevy
** File description:
** Editor Profiling Windows
*/

#pragma once

#include "EditorWindow.hpp"
#include "ProfilerWindow.hpp"
#include "glWindow.hpp"
#include <chrono>

#include <string>

namespace cevy::editor {

class ProfilingWindow : public EditorWindow {
  std::vector<float> frames;
  std::vector<float> framesAverage;
  legit::ProfilerWindow legitProfiler;
  std::chrono::high_resolution_clock::time_point lastCall;

  public:
  ProfilingWindow() : EditorWindow(true, "Profiling") { legitProfiler = legit::ProfilerWindow(); }

  void render(cevy::editor::Editor &editor, glWindow &glwindow, cevy::ecs::World &world) override;
};
}; // namespace cevy::editor
