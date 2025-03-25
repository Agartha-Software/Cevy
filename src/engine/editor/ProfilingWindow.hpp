/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Editor Profiling Windows
*/

#pragma once

#include "ProfilerWindow.hpp"
#include "glWindow.hpp"
#include "EditorWindow.hpp"
#include <chrono>

#include <string>


namespace cevy::editor {

class ProfilingWindow : public EditorWindow {
    std::vector<float> frames;
    std::vector<float> framesAverage;
    legit::ProfilerWindow legitProfiler;
    std::chrono::high_resolution_clock::time_point last_call;

    public:
    ProfilingWindow() : EditorWindow(true, "Profiling") {
      legitProfiler = legit::ProfilerWindow();
    }

    void render(cevy::editor::Editor &editor, glWindow &glwindow, cevy::ecs::World &world) override;
  };
};
