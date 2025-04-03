/*
The Legit profiler is based on this project:
https://github.com/Raikiri/LegitProfiler
by Raikiri
It's been heavily modified to fit our needs and follows our code guidelines.
*/

#include "ProfilerWindow.hpp"
#include "ProfilerGraph.hpp"
#include "imgui.h"
#include <algorithm>
#include <chrono>
#include <glm/fwd.hpp>
#include <glm/gtx/string_cast.hpp>

void legit::ProfilerWindow::render() {
  this->fpsFramesCount++;
  auto curr_frame_time = std::chrono::system_clock::now();
  float fps_delta_time =
      std::chrono::duration<float>(curr_frame_time - this->prevFpsFrameTime).count();
  if (fps_delta_time > 0.5f) {
    this->avgFrameTime = fps_delta_time / float(this->fpsFramesCount);
    this->fpsFramesCount = 0;
    this->prevFpsFrameTime = curr_frame_time;
  }

  ImVec2 canvas_size = ImGui::GetContentRegionAvail();

  int max_graph_height = 300;
  int available_graph_height = (int(canvas_size.y) - int(ImGui::GetStyle().ItemSpacing.y)) / 2;
  int graph_height = std::min(max_graph_height, available_graph_height);
  int legend_width = 235;
  int graph_width = int(canvas_size.x) - legend_width;
  gpuGraph.renderTimings(graph_width, legend_width, graph_height, this->frameOffset);
  cpuGraph.renderTimings(graph_width, legend_width, graph_height, this->frameOffset);

  ImGui::Columns(2);
  ImGui::Checkbox("Stop profiling", &this->stopProfiling);

  // ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - textSize);
  ImGui::Checkbox("Colored legend text", &this->useColoredLegendText);
  ImGui::DragInt("Frame offset", &this->frameOffset, 1.0f, 0, 400);
  ImGui::NextColumn();

  ImGui::SliderInt("Frame width", &this->frameWidth, 1, 4);
  ImGui::SliderInt("Frame spacing", &this->frameSpacing, 0, 2);
  ImGui::Columns(1);
  //    }
  if (!this->stopProfiling) {
    this->frameOffset = 0;
  }

  gpuGraph.frameWidth = this->frameWidth;
  gpuGraph.frameSpacing = this->frameSpacing;
  gpuGraph.useColoredLegendText = this->useColoredLegendText;
  gpuGraph.stopProfiling = this->stopProfiling;

  cpuGraph.frameWidth = this->frameWidth;
  cpuGraph.frameSpacing = this->frameSpacing;
  cpuGraph.useColoredLegendText = this->useColoredLegendText;
  cpuGraph.stopProfiling = this->stopProfiling;
}
