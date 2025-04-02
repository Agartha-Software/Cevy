/*
The Legit profiler is based on this project:
https://github.com/Raikiri/LegitProfiler
by Raikiri
It's been heavily modified to fit our needs and follows our code guidelines.
*/

#pragma once

#include "ProfilerGraph.hpp"
#include <chrono>
#include <glm/fwd.hpp>
#include <glm/gtx/string_cast.hpp>

namespace legit {
class ProfilerWindow {
  public:
  ProfilerWindow() : cpuGraph(300), gpuGraph(300) {
    stopProfiling = false;
    frameOffset = 0;
    frameWidth = 3;
    frameSpacing = 1;
    useColoredLegendText = true;
    prevFpsFrameTime = std::chrono::system_clock::now();
    fpsFramesCount = 0;
    avgFrameTime = 1.0f;
  }

  void render();

  bool stopProfiling;
  int frameOffset;
  ProfilerGraph cpuGraph;
  ProfilerGraph gpuGraph;
  int frameWidth;
  int frameSpacing;
  bool useColoredLegendText;
  using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
  TimePoint prevFpsFrameTime;
  size_t fpsFramesCount;
  float avgFrameTime;
};
} // namespace legit
