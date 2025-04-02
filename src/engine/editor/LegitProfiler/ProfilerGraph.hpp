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

  ProfilerGraph(size_t frames_count);

  void loadFrameData(const legit::ProfilerTask *tasks, size_t count);
  void renderTimings(int graph_width, int legend_width, int height, int frame_index_offset);

  private:
  void rebuildTaskStats(size_t end_frame, size_t frames_count);
  void renderGraph(ImDrawList *draw_list, glm::vec2 graph_pos, glm::vec2 graph_size,
                   size_t frame_index_offset);

  void renderLegend(ImDrawList *draw_list, glm::vec2 legend_pos, glm::vec2 legend_size,
                    size_t frame_index_offset);
};
}
