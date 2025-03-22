/*
The Legit profiler is based on this project:
https://github.com/Raikiri/LegitProfiler
by Raikiri
It's been heavily modified to fit our needs and follows our code guidelines.
*/

#pragma once

#include "ProfilerTask.hpp"
#include "imgui.h"
#include <glm/fwd.hpp>
#include <glm/gtx/string_cast.hpp>
#include <map>
#include <vector>


namespace legit {

class ProfilerGraph {
  private:
  struct FrameData {
    std::vector<legit::ProfilerTask> tasks;
    std::vector<size_t> taskStatsIndex;
  };

  struct TaskStats {
    double maxTime;
    size_t priorityOrder;
    size_t onScreenIndex;
  };

  std::vector<TaskStats> taskStats;
  std::map<std::string, size_t> taskNameToStatsIndex;

  std::vector<FrameData> frames;
  size_t currFrameIndex = 0;

  public:
  int frameWidth;
  int frameSpacing;
  bool useColoredLegendText;
  float maxFrameTime = 1.0f / 30.0f;
  bool stopProfiling;

  ProfilerGraph(size_t framesCount);

  void LoadFrameData(const legit::ProfilerTask *tasks, size_t count);
  void RenderTimings(int graphWidth, int legendWidth, int height, int frameIndexOffset);

  private:
  void RebuildTaskStats(size_t endFrame, size_t framesCount);
  void RenderGraph(ImDrawList *drawList, glm::vec2 graphPos, glm::vec2 graphSize,
                   size_t frameIndexOffset);

  void RenderLegend(ImDrawList *drawList, glm::vec2 legendPos, glm::vec2 legendSize,
                    size_t frameIndexOffset);
};
}
