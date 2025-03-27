/*
The Legit profiler is based on this project:
https://github.com/Raikiri/LegitProfiler
by Raikiri
It's been heavily modified to fit our needs and follows our code guidelines.
*/

#include "imgui.h"
#include "ProfilerGraph.hpp"
#include "ProfilerWindow.hpp"
#include <algorithm>
#include <chrono>
#include <glm/fwd.hpp>
#include <glm/gtx/string_cast.hpp>

void legit::ProfilerWindow::Render() {
  fpsFramesCount++;
  auto currFrameTime = std::chrono::system_clock::now();
  {
    float fpsDeltaTime = std::chrono::duration<float>(currFrameTime - prevFpsFrameTime).count();
    if (fpsDeltaTime > 0.5f) {
      this->avgFrameTime = fpsDeltaTime / float(fpsFramesCount);
      fpsFramesCount = 0;
      prevFpsFrameTime = currFrameTime;
    }
  }

  ImVec2 canvasSize = ImGui::GetContentRegionAvail();

  int sizeMargin = int(ImGui::GetStyle().ItemSpacing.y);
  int maxGraphHeight = 300;
  int availableGraphHeight = (int(canvasSize.y) - sizeMargin) / 2;
  int graphHeight = std::min(maxGraphHeight, availableGraphHeight);
  int legendWidth = 235;
  int graphWidth = int(canvasSize.x) - legendWidth;
  gpuGraph.RenderTimings(graphWidth, legendWidth, graphHeight, frameOffset);
  cpuGraph.RenderTimings(graphWidth, legendWidth, graphHeight, frameOffset);
  // removed due
//    if (graphHeight * 2 + sizeMargin + sizeMargin < canvasSize.y) {
  ImGui::Columns(2);
  ImGui::Checkbox("Stop profiling", &stopProfiling);
  cpuGraph.stopProfiling = stopProfiling;
  gpuGraph.stopProfiling = stopProfiling;
  // ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - textSize);
  ImGui::Checkbox("Colored legend text", &useColoredLegendText);
  ImGui::DragInt("Frame offset", &frameOffset, 1.0f, 0, 400);
  ImGui::NextColumn();

  ImGui::SliderInt("Frame width", &frameWidth, 1, 4);
  ImGui::SliderInt("Frame spacing", &frameSpacing, 0, 2);
  ImGui::Columns(1);
//    }
  if (!stopProfiling) {
    frameOffset = 0;
  }

  gpuGraph.frameWidth = frameWidth;
  gpuGraph.frameSpacing = frameSpacing;
  gpuGraph.useColoredLegendText = useColoredLegendText;
  cpuGraph.frameWidth = frameWidth;
  cpuGraph.frameSpacing = frameSpacing;
  cpuGraph.useColoredLegendText = useColoredLegendText;
}
