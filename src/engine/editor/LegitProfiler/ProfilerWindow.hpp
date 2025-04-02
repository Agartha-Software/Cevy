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
  using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

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

  ProfilerGraph cpuGraph;
  ProfilerGraph gpuGraph;

  protected:
  int frameOffset;
  int frameWidth;
  int frameSpacing;

  TimePoint prevFpsFrameTime;
  size_t fpsFramesCount;
  float avgFrameTime;
  bool useColoredLegendText;

  bool stopProfiling;
};
} // namespace legit
