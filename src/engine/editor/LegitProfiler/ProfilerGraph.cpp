/*
The Legit profiler is based on this project:
https://github.com/Raikiri/LegitProfiler
by Raikiri
It's been heavily modified to fit our needs and follows our code guidelines.
*/

#include "ProfilerGraph.hpp"
#include "ProfilerTask.hpp"
#include "imgui.h"
#include "legitProfiler.hpp"
#include <algorithm>
#include <array>
#include <glm/fwd.hpp>
#include <glm/gtx/string_cast.hpp>
#include <ios>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

legit::ProfilerGraph::ProfilerGraph(size_t frames_count) : stopProfiling(false) {
  this->frames.resize(frames_count);
  for (auto &frame : this->frames) {
    frame.tasks.reserve(100);
  }
  this->frameWidth = 3;
  this->frameSpacing = 1;
  this->useColoredLegendText = false;
}

void legit::ProfilerGraph::loadFrameData(const legit::ProfilerTask *tasks, size_t count) {
  if (this->stopProfiling) {
    return;
  }

  auto &curr_frame = this->frames[this->currFrameIndex];
  curr_frame.tasks.resize(0);
  for (size_t task_index = 0; task_index < count; task_index++) {
    if (task_index == 0)
      curr_frame.tasks.push_back(tasks[task_index]);
    else {
      if (tasks[task_index - 1].color != tasks[task_index].color ||
          tasks[task_index - 1].name != tasks[task_index].name) {
        curr_frame.tasks.push_back(tasks[task_index]);
      } else {
        curr_frame.tasks.back().endTime = tasks[task_index].endTime;
      }
    }
  }
  curr_frame.taskStatsIndex.resize(curr_frame.tasks.size());

  for (size_t task_index = 0; task_index < curr_frame.tasks.size(); task_index++) {
    auto &task = curr_frame.tasks[task_index];
    auto it = this->taskNameToStatsIndex.find(task.name);
    if (it == this->taskNameToStatsIndex.end()) {
      this->taskNameToStatsIndex[task.name] = this->taskStats.size();
      TaskStats taskStat;
      taskStat.maxTime = -1.0;
      this->taskStats.push_back(taskStat);
    }
    curr_frame.taskStatsIndex[task_index] = this->taskNameToStatsIndex[task.name];
  }
  this->currFrameIndex = (this->currFrameIndex + 1) % this->frames.size();

  rebuildTaskStats(this->currFrameIndex, 300);
}

using namespace legit;

static void renderTaskMarker(ImDrawList *draw_list, glm::vec2 left_min_point,
                             glm::vec2 left_max_point, glm::vec2 right_min_point,
                             glm::vec2 right_max_point, uint32_t col) {
  draw_list->AddRectFilled(to_im_vec(left_min_point), to_im_vec(left_max_point), col);
  draw_list->AddRectFilled(to_im_vec(right_min_point), to_im_vec(right_max_point), col);
  std::array<ImVec2, 4> points = {
      ImVec2(left_max_point.x, left_min_point.y), ImVec2(left_max_point.x, left_max_point.y),
      ImVec2(right_min_point.x, right_max_point.y), ImVec2(right_min_point.x, right_min_point.y)};
  draw_list->AddConvexPolyFilled(points.data(), int(points.size()), col);
}

void legit::ProfilerGraph::renderTimings(int graph_width, int legend_width, int height,
                                         int frame_index_offset) {
  ImDrawList *draw_list = ImGui::GetWindowDrawList();
  const glm::vec2 widgetPos = to_glm_vec(ImGui::GetCursorScreenPos());
  this->renderGraph(draw_list, widgetPos, glm::vec2(graph_width, height), frame_index_offset);
  this->renderLegend(draw_list, widgetPos + glm::vec2(graph_width, 0.0f),
                     glm::vec2(legend_width, height), frame_index_offset);
  ImGui::Dummy(ImVec2(float(graph_width + legend_width), float(height)));
}

void legit::ProfilerGraph::rebuildTaskStats(size_t end_frame, size_t frames_count) {
  for (auto &taskStat : this->taskStats) {
    taskStat.maxTime = -1.0f;
    taskStat.priorityOrder = -1;
    taskStat.onScreenIndex = -1;
  }

  for (size_t frameNumber = 0; frameNumber < frames_count; frameNumber++) {
    size_t frameIndex = (end_frame - 1 - frameNumber + this->frames.size()) % this->frames.size();
    auto &frame = this->frames[frameIndex];
    for (size_t task_index = 0; task_index < frame.tasks.size(); task_index++) {
      auto &task = frame.tasks[task_index];
      auto &stats = this->taskStats[frame.taskStatsIndex[task_index]];
      stats.maxTime = std::max(stats.maxTime, task.endTime - task.startTime);
    }
  }
  std::vector<size_t> stat_priorities;
  stat_priorities.resize(this->taskStats.size());
  for (size_t statIndex = 0; statIndex < this->taskStats.size(); statIndex++)
    stat_priorities[statIndex] = statIndex;

  std::sort(stat_priorities.begin(), stat_priorities.end(), [this](size_t left, size_t right) {
    return this->taskStats[left].maxTime > this->taskStats[right].maxTime;
  });
  for (size_t statNumber = 0; statNumber < taskStats.size(); statNumber++) {
    size_t statIndex = stat_priorities[statNumber];
    taskStats[statIndex].priorityOrder = statNumber;
  }
}

void legit::ProfilerGraph::renderGraph(ImDrawList *draw_list, glm::vec2 graph_pos,
                                       glm::vec2 graph_size, size_t frame_index_offset) {
  draw_list->AddRect(to_im_vec(graph_pos), to_im_vec(graph_pos + graph_size), 0xffffffff);
  float heightThreshold = 1.0f;

  for (size_t frameNumber = 0; frameNumber < this->frames.size(); frameNumber++) {
    size_t frameIndex =
        (this->currFrameIndex - frame_index_offset - 1 - frameNumber + 2 * this->frames.size()) %
        this->frames.size();

    glm::vec2 framePos =
        graph_pos + glm::vec2(graph_size.x - 1 - this->frameWidth -
                                  (this->frameWidth + this->frameSpacing) * frameNumber,
                              graph_size.y - 1);
    if (framePos.x < graph_pos.x + 1)
      break;
    glm::vec2 taskPos = framePos + glm::vec2(0.0f, 0.0f);
    auto &frame = this->frames[frameIndex];
    for (const auto &task : frame.tasks) {
      float task_start_height = task.startTime * graph_size.y / (this->maxFrameTime * 1000);
      float task_end_height = task.endTime * graph_size.y / (this->maxFrameTime * 1000);

      if (std::abs(task_end_height - task_start_height) > heightThreshold) {
        draw_list->AddRectFilled(to_im_vec(taskPos + glm::vec2(0.0f, -task_start_height)),
                                 to_im_vec(taskPos + glm::vec2(this->frameWidth, -task_end_height)),
                                 task.color);
      }
    }
  }
}

void legit::ProfilerGraph::renderLegend(ImDrawList *draw_list, glm::vec2 legend_pos,
                                        glm::vec2 legend_size, size_t frame_index_offset) {
  float marker_left_rect_margin = 3.0f;
  float marker_left_rect_width = 5.0f;
  float marker_mid_width = 30.0f;
  float marker_right_rect_width = 10.0f;
  float marker_rigth_rect_margin = 3.0f;
  float marker_right_rect_height = 10.0f;
  float marker_right_rect_spacing = 4.0f;
  float name_offset = 50.0f;
  glm::vec2 text_margin = glm::vec2(5.0f, -3.0f);

  auto &curr_frame =
      this->frames[(this->currFrameIndex - frame_index_offset - 1 + 2 * this->frames.size()) %
                   this->frames.size()];
  size_t max_tasks_count = legend_size.y / (marker_right_rect_height + marker_right_rect_spacing);

  for (auto &task_stat : this->taskStats) {
    task_stat.onScreenIndex = -1;
  }

  size_t tasks_to_show = std::min<size_t>(this->taskStats.size(), max_tasks_count);
  size_t tasks_shown_count = 0;
  for (size_t task_index = 0; task_index < curr_frame.tasks.size(); task_index++) {
    auto &task = curr_frame.tasks[task_index];
    auto &stat = this->taskStats[curr_frame.taskStatsIndex[task_index]];

    if (stat.priorityOrder >= tasks_to_show)
      continue;

    if (stat.onScreenIndex == size_t(-1)) {
      stat.onScreenIndex = tasks_shown_count++;
    } else
      continue;
    float task_start_height = task.startTime * legend_size.y / (this->maxFrameTime * 1000);
    float task_end_height = task.endTime * legend_size.y / (this->maxFrameTime * 1000);

    glm::vec2 marker_left_rect_min = legend_pos + glm::vec2(marker_left_rect_margin, legend_size.y);
    glm::vec2 marker_left_rect_max = marker_left_rect_min + glm::vec2(marker_left_rect_width, 0.0f);
    marker_left_rect_min.y -= task_start_height;
    marker_left_rect_max.y -= task_end_height;

    glm::vec2 marker_right_rect_min =
        legend_pos +
        glm::vec2(marker_left_rect_margin + marker_left_rect_width + marker_mid_width,
                  legend_size.y - marker_rigth_rect_margin -
                      (marker_right_rect_height + marker_right_rect_spacing) * stat.onScreenIndex);
    glm::vec2 marker_right_rect_max =
        marker_right_rect_min + glm::vec2(marker_right_rect_width, -marker_right_rect_height);
    renderTaskMarker(draw_list, marker_left_rect_min, marker_left_rect_max, marker_right_rect_min,
                     marker_right_rect_max, task.color);

    uint32_t textColor = this->useColoredLegendText ? task.color : legit::Colors::imguiText;

    float taskTimeMs = float(task.endTime - task.startTime);
    std::ostringstream time_text;
    time_text.precision(2);
    time_text << std::fixed << "[" << (taskTimeMs);

    draw_list->AddText(to_im_vec(marker_right_rect_max + text_margin), textColor,
                       time_text.str().c_str());
    draw_list->AddText(
        to_im_vec(marker_right_rect_max + text_margin + glm::vec2(name_offset, 0.0f)), textColor,
        (std::string("ms] ") + task.name).c_str());
  }
}
