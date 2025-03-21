/*

*/

#pragma once

#include "legitProfiler.hpp"
#include "ProfilerTask.hpp"
#include "imgui.h"
#include <algorithm>
#include <array>
#include <glm/fwd.hpp>
#include <glm/gtx/string_cast.hpp>
#include <ios>
#include <iostream>
#include <map>
#include <sstream>
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

  ProfilerGraph(size_t framesCount) : stopProfiling(false) {
    frames.resize(framesCount);
    for (auto &frame : frames)
      frame.tasks.reserve(100);
    frameWidth = 3;
    frameSpacing = 1;
    useColoredLegendText = false;
  }

  void LoadFrameData(const legit::ProfilerTask *tasks, size_t count) {
    if (stopProfiling)
      return;

    auto &currFrame = frames[currFrameIndex];
    currFrame.tasks.resize(0);
    for (size_t taskIndex = 0; taskIndex < count; taskIndex++) {
      if (taskIndex == 0)
        currFrame.tasks.push_back(tasks[taskIndex]);
      else {
        if (tasks[taskIndex - 1].color != tasks[taskIndex].color ||
            tasks[taskIndex - 1].name != tasks[taskIndex].name) {
          currFrame.tasks.push_back(tasks[taskIndex]);
        } else {
          currFrame.tasks.back().endTime = tasks[taskIndex].endTime;
        }
      }
    }
    currFrame.taskStatsIndex.resize(currFrame.tasks.size());

    for (size_t taskIndex = 0; taskIndex < currFrame.tasks.size(); taskIndex++) {
      auto &task = currFrame.tasks[taskIndex];
      auto it = taskNameToStatsIndex.find(task.name);
      if (it == taskNameToStatsIndex.end()) {
        taskNameToStatsIndex[task.name] = taskStats.size();
        TaskStats taskStat;
        taskStat.maxTime = -1.0;
        taskStats.push_back(taskStat);
      }
      currFrame.taskStatsIndex[taskIndex] = taskNameToStatsIndex[task.name];
    }
    currFrameIndex = (currFrameIndex + 1) % frames.size();

    RebuildTaskStats(currFrameIndex, 300 /*frames.size()*/);
  }

  void RenderTimings(int graphWidth, int legendWidth, int height, int frameIndexOffset) {
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    const glm::vec2 widgetPos = to_glm_vec(ImGui::GetCursorScreenPos());
    RenderGraph(drawList, widgetPos, glm::vec2(graphWidth, height), frameIndexOffset);
    RenderLegend(drawList, widgetPos + glm::vec2(graphWidth, 0.0f), glm::vec2(legendWidth, height),
                 frameIndexOffset);
    ImGui::Dummy(ImVec2(float(graphWidth + legendWidth), float(height)));
  }

  private:
  void RebuildTaskStats(size_t endFrame, size_t framesCount) {
    for (auto &taskStat : taskStats) {
      taskStat.maxTime = -1.0f;
      taskStat.priorityOrder = size_t(-1);
      taskStat.onScreenIndex = size_t(-1);
    }

    for (size_t frameNumber = 0; frameNumber < framesCount; frameNumber++) {
      size_t frameIndex = (endFrame - 1 - frameNumber + frames.size()) % frames.size();
      auto &frame = frames[frameIndex];
      for (size_t taskIndex = 0; taskIndex < frame.tasks.size(); taskIndex++) {
        auto &task = frame.tasks[taskIndex];
        auto &stats = taskStats[frame.taskStatsIndex[taskIndex]];
        stats.maxTime = std::max(stats.maxTime, task.endTime - task.startTime);
      }
    }
    std::vector<size_t> statPriorities;
    statPriorities.resize(taskStats.size());
    for (size_t statIndex = 0; statIndex < taskStats.size(); statIndex++)
      statPriorities[statIndex] = statIndex;

    std::sort(statPriorities.begin(), statPriorities.end(), [this](size_t left, size_t right) {
      return taskStats[left].maxTime > taskStats[right].maxTime;
    });
    for (size_t statNumber = 0; statNumber < taskStats.size(); statNumber++) {
      size_t statIndex = statPriorities[statNumber];
      taskStats[statIndex].priorityOrder = statNumber;
    }
  }

  void RenderGraph(ImDrawList *drawList, glm::vec2 graphPos, glm::vec2 graphSize,
                   size_t frameIndexOffset) {
    Rect(drawList, graphPos, graphPos + graphSize, 0xffffffff, false);
    float heightThreshold = 1.0f;

    for (size_t frameNumber = 0; frameNumber < frames.size(); frameNumber++) {
      size_t frameIndex =
          (currFrameIndex - frameIndexOffset - 1 - frameNumber + 2 * frames.size()) % frames.size();

      glm::vec2 framePos = graphPos + glm::vec2(graphSize.x - 1 - frameWidth -
                                                    (frameWidth + frameSpacing) * frameNumber,
                                                graphSize.y - 1);
      if (framePos.x < graphPos.x + 1)
        break;
      glm::vec2 taskPos = framePos + glm::vec2(0.0f, 0.0f);
      auto &frame = frames[frameIndex];
      for (const auto &task : frame.tasks) {
        float taskStartHeight = task.startTime * graphSize.y / (maxFrameTime * 1000);
        float taskEndHeight = task.endTime * graphSize.y / (maxFrameTime * 1000);

        if (std::abs(taskEndHeight - taskStartHeight) > heightThreshold) {
          Rect(drawList, taskPos + glm::vec2(0.0f, -taskStartHeight),
              taskPos + glm::vec2(frameWidth, -taskEndHeight), task.color, true);
        }
      }
    }
  }

  void RenderLegend(ImDrawList *drawList, glm::vec2 legendPos, glm::vec2 legendSize,
                    size_t frameIndexOffset) {
    float markerLeftRectMargin = 3.0f;
    float markerLeftRectWidth = 5.0f;
    float markerMidWidth = 30.0f;
    float markerRightRectWidth = 10.0f;
    float markerRigthRectMargin = 3.0f;
    float markerRightRectHeight = 10.0f;
    float markerRightRectSpacing = 4.0f;
    float nameOffset = 50.0f;
    glm::vec2 textMargin = glm::vec2(5.0f, -3.0f);

    auto &currFrame =
        frames[(currFrameIndex - frameIndexOffset - 1 + 2 * frames.size()) % frames.size()];
    size_t maxTasksCount = size_t(legendSize.y / (markerRightRectHeight + markerRightRectSpacing));

    for (auto &taskStat : taskStats) {
      taskStat.onScreenIndex = size_t(-1);
    }

    size_t tasksToShow = std::min<size_t>(taskStats.size(), maxTasksCount);
    size_t tasksShownCount = 0;
    for (size_t taskIndex = 0; taskIndex < currFrame.tasks.size(); taskIndex++) {
      auto &task = currFrame.tasks[taskIndex];
      auto &stat = taskStats[currFrame.taskStatsIndex[taskIndex]];

      if (stat.priorityOrder >= tasksToShow)
        continue;

      if (stat.onScreenIndex == size_t(-1)) {
        stat.onScreenIndex = tasksShownCount++;
      } else
        continue;
      float taskStartHeight = task.startTime * legendSize.y / (maxFrameTime * 1000);
      float taskEndHeight = task.endTime * legendSize.y / (maxFrameTime * 1000);

      glm::vec2 markerLeftRectMin = legendPos + glm::vec2(markerLeftRectMargin, legendSize.y);
      glm::vec2 markerLeftRectMax = markerLeftRectMin + glm::vec2(markerLeftRectWidth, 0.0f);
      markerLeftRectMin.y -= taskStartHeight;
      markerLeftRectMax.y -= taskEndHeight;

      glm::vec2 markerRightRectMin =
          legendPos +
          glm::vec2(markerLeftRectMargin + markerLeftRectWidth + markerMidWidth,
                    legendSize.y - markerRigthRectMargin -
                        (markerRightRectHeight + markerRightRectSpacing) * stat.onScreenIndex
                    );
      glm::vec2 markerRightRectMax =
          markerRightRectMin + glm::vec2(markerRightRectWidth, -markerRightRectHeight);
      RenderTaskMarker(drawList, markerLeftRectMin, markerLeftRectMax, markerRightRectMin,
                       markerRightRectMax, task.color);

      uint32_t textColor =
          useColoredLegendText ? task.color : legit::Colors::imguiText;

      float taskTimeMs = float(task.endTime - task.startTime);
      std::ostringstream timeText;
      timeText.precision(2);
      timeText << std::fixed << std::string("[") << (taskTimeMs);

      Text(drawList, markerRightRectMax + textMargin, textColor, timeText.str().c_str());
      Text(drawList, markerRightRectMax + textMargin + glm::vec2(nameOffset, 0.0f), textColor,
           (std::string("ms] ") + task.name).c_str());
    }
  }

  static void Rect(ImDrawList *drawList, glm::vec2 minPoint, glm::vec2 maxPoint, uint32_t col,
                   bool filled = true) {
    if (filled)
      drawList->AddRectFilled(ImVec2(minPoint.x, minPoint.y), ImVec2(maxPoint.x, maxPoint.y), col);
    else
      drawList->AddRect(ImVec2(minPoint.x, minPoint.y), ImVec2(maxPoint.x, maxPoint.y), col);
  }

  static void Text(ImDrawList *drawList, glm::vec2 point, uint32_t col, const char *text) {
    drawList->AddText(ImVec2(point.x, point.y), col, text);
  }

  static void Triangle(ImDrawList *drawList, std::array<glm::vec2, 3> points, uint32_t col,
                       bool filled = true) {
    if (filled)
      drawList->AddTriangleFilled(ImVec2(points[0].x, points[0].y),
                                  ImVec2(points[1].x, points[1].y),
                                  ImVec2(points[2].x, points[2].y), col);
    else
      drawList->AddTriangle(ImVec2(points[0].x, points[0].y), ImVec2(points[1].x, points[1].y),
                            ImVec2(points[2].x, points[2].y), col);
  }

  static void RenderTaskMarker(ImDrawList *drawList, glm::vec2 leftMinPoint, glm::vec2 leftMaxPoint,
                               glm::vec2 rightMinPoint, glm::vec2 rightMaxPoint, uint32_t col) {
    Rect(drawList, leftMinPoint, leftMaxPoint, col, true);
    Rect(drawList, rightMinPoint, rightMaxPoint, col, true);
    std::array<ImVec2, 4> points = {
        ImVec2(leftMaxPoint.x, leftMinPoint.y), ImVec2(leftMaxPoint.x, leftMaxPoint.y),
        ImVec2(rightMinPoint.x, rightMaxPoint.y), ImVec2(rightMinPoint.x, rightMinPoint.y)};
    drawList->AddConvexPolyFilled(points.data(), int(points.size()), col);
  }
};
}
