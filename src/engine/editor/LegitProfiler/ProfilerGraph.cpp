/*
The Legit profiler is based on this project:
https://github.com/Raikiri/LegitProfiler
by Raikiri
It's been heavily modified to fit our needs and follows our code guidelines.
*/

#include "legitProfiler.hpp"
#include "ProfilerTask.hpp"
#include "ProfilerGraph.hpp"
#include "imgui.h"
#include <algorithm>
#include <array>
#include <glm/fwd.hpp>
#include <ios>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

legit::ProfilerGraph::ProfilerGraph(size_t framesCount) : stopProfiling(false) {
    this->frames.resize(framesCount);
    for (auto &frame : this->frames) {
        frame.tasks.reserve(100);
    }
    this->frameWidth = 3;
    this->frameSpacing = 1;
    this->useColoredLegendText = false;
}

void legit::ProfilerGraph::LoadFrameData(const legit::ProfilerTask *tasks, size_t count) {
  if (this->stopProfiling) {
    return;
  }

  auto &currFrame = this->frames[this->currFrameIndex];
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
    auto it = this->taskNameToStatsIndex.find(task.name);
    if (it == this->taskNameToStatsIndex.end()) {
      this->taskNameToStatsIndex[task.name] = taskStats.size();
      TaskStats taskStat;
      taskStat.maxTime = -1.0;
      this->taskStats.push_back(taskStat);
    }
    currFrame.taskStatsIndex[taskIndex] = this->taskNameToStatsIndex[task.name];
  }
  this->currFrameIndex = (this->currFrameIndex + 1) % this->frames.size();

  RebuildTaskStats(this->currFrameIndex, 300);
}

using namespace legit;

static void RenderTaskMarker(ImDrawList *drawList, glm::vec2 leftMinPoint, glm::vec2 leftMaxPoint,
                              glm::vec2 rightMinPoint, glm::vec2 rightMaxPoint, uint32_t col) {
  drawList->AddRectFilled(to_im_vec(leftMinPoint), to_im_vec(leftMaxPoint), col);
  drawList->AddRectFilled(to_im_vec(rightMinPoint), to_im_vec(rightMaxPoint), col);
  std::array<ImVec2, 4> points = {
      ImVec2(leftMaxPoint.x, leftMinPoint.y), ImVec2(leftMaxPoint.x, leftMaxPoint.y),
      ImVec2(rightMinPoint.x, rightMaxPoint.y), ImVec2(rightMinPoint.x, rightMinPoint.y)};
  drawList->AddConvexPolyFilled(points.data(), int(points.size()), col);
}

void legit::ProfilerGraph::RenderTimings(int graphWidth, int legendWidth, int height, int frameIndexOffset) {
  ImDrawList *drawList = ImGui::GetWindowDrawList();
  const glm::vec2 widgetPos = to_glm_vec(ImGui::GetCursorScreenPos());
  this->RenderGraph(drawList, widgetPos, glm::vec2(graphWidth, height), frameIndexOffset);
  this->RenderLegend(drawList, widgetPos + glm::vec2(graphWidth, 0.0f), glm::vec2(legendWidth, height),
                frameIndexOffset);
  ImGui::Dummy(ImVec2(float(graphWidth + legendWidth), float(height)));
}

void legit::ProfilerGraph::RebuildTaskStats(size_t endFrame, size_t framesCount) {
  for (auto &taskStat : this->taskStats) {
    taskStat.maxTime = -1.0f;
    taskStat.priorityOrder = -1;
    taskStat.onScreenIndex = -1;
  }

  for (size_t frameNumber = 0; frameNumber < framesCount; frameNumber++) {
    size_t frameIndex = (endFrame - 1 - frameNumber + this->frames.size()) % this->frames.size();
    auto &frame = this->frames[frameIndex];
    for (size_t taskIndex = 0; taskIndex < frame.tasks.size(); taskIndex++) {
      auto &task = frame.tasks[taskIndex];
      auto &stats = this->taskStats[frame.taskStatsIndex[taskIndex]];
      stats.maxTime = std::max(stats.maxTime, task.endTime - task.startTime);
    }
  }
  std::vector<size_t> statPriorities;
  statPriorities.resize(this->taskStats.size());
  for (size_t statIndex = 0; statIndex < this->taskStats.size(); statIndex++)
    statPriorities[statIndex] = statIndex;

  std::sort(statPriorities.begin(), statPriorities.end(), [this](size_t left, size_t right) {
    return this->taskStats[left].maxTime > this->taskStats[right].maxTime;
  });
  for (size_t statNumber = 0; statNumber < taskStats.size(); statNumber++) {
    size_t statIndex = statPriorities[statNumber];
    taskStats[statIndex].priorityOrder = statNumber;
  }
}

void legit::ProfilerGraph::RenderGraph(ImDrawList *drawList, glm::vec2 graphPos, glm::vec2 graphSize, size_t frameIndexOffset) {
  drawList->AddRect(to_im_vec(graphPos), to_im_vec(graphPos + graphSize), 0xffffffff);
  float heightThreshold = 1.0f;

  for (size_t frameNumber = 0; frameNumber < this->frames.size(); frameNumber++) {
    size_t frameIndex =
        (this->currFrameIndex - frameIndexOffset - 1 - frameNumber + 2 * this->frames.size()) % this->frames.size();

    glm::vec2 framePos = graphPos + glm::vec2(graphSize.x - 1 - this->frameWidth -
                                                  (this->frameWidth + this->frameSpacing) * frameNumber,
                                              graphSize.y - 1);
    if (framePos.x < graphPos.x + 1)
      break;
    glm::vec2 taskPos = framePos + glm::vec2(0.0f, 0.0f);
    auto &frame = this->frames[frameIndex];
    for (const auto &task : frame.tasks) {
      float taskStartHeight = task.startTime * graphSize.y / (this->maxFrameTime * 1000);
      float taskEndHeight = task.endTime * graphSize.y / (this->maxFrameTime * 1000);

      if (std::abs(taskEndHeight - taskStartHeight) > heightThreshold) {
        drawList->AddRectFilled(to_im_vec(taskPos + glm::vec2(0.0f, -taskStartHeight)),
          to_im_vec(taskPos + glm::vec2(this->frameWidth, -taskEndHeight)), task.color);
      }
    }
  }
}

void legit::ProfilerGraph::RenderLegend(ImDrawList *drawList, glm::vec2 legendPos, glm::vec2 legendSize,
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
      this->frames[(this->currFrameIndex - frameIndexOffset - 1 + 2 * this->frames.size()) % this->frames.size()];
  size_t maxTasksCount = legendSize.y / (markerRightRectHeight + markerRightRectSpacing);

  for (auto &taskStat : this->taskStats) {
    taskStat.onScreenIndex = -1;
  }

  size_t tasksToShow = std::min<size_t>(this->taskStats.size(), maxTasksCount);
  size_t tasksShownCount = 0;
  for (size_t taskIndex = 0; taskIndex < currFrame.tasks.size(); taskIndex++) {
    auto &task = currFrame.tasks[taskIndex];
    auto &stat = this->taskStats[currFrame.taskStatsIndex[taskIndex]];

    if (stat.priorityOrder >= tasksToShow)
      continue;

    if (stat.onScreenIndex == size_t(-1)) {
      stat.onScreenIndex = tasksShownCount++;
    } else
      continue;
    float taskStartHeight = task.startTime * legendSize.y / (this->maxFrameTime * 1000);
    float taskEndHeight = task.endTime * legendSize.y / (this->maxFrameTime * 1000);

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
        this->useColoredLegendText ? task.color : legit::Colors::imguiText;

    float taskTimeMs = float(task.endTime - task.startTime);
    std::ostringstream timeText;
    timeText.precision(2);
    timeText << std::fixed << "[" << (taskTimeMs);

    drawList->AddText(to_im_vec(markerRightRectMax + textMargin), textColor, timeText.str().c_str());
    drawList->AddText(to_im_vec(markerRightRectMax + textMargin + glm::vec2(nameOffset, 0.0f)), textColor, (std::string("ms] ") + task.name).c_str());
  }
}
