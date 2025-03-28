/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Editor Profiling Windows
*/

#include <cstddef>
#include <ostream>
#include <typeindex>
#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL

#include "ProfilingWindow.hpp"
#include "Editor.hpp"
#include "ProfilerTask.hpp"
#include "Scheduler.hpp"
#include "imgui.h"
#include <chrono>

#include <algorithm>
#include <unordered_map>

static glm::vec3 hsv2rgb(glm::vec3 c) {
  glm::vec4 K = glm::vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  glm::vec3 p = abs(fract(c.xxx() + K.xyz()) * 6.0f - K.www());
  return c.z * mix(K.xxx(), clamp(p - K.xxx(), 0.0f, 1.0f), c.y);
}

// unsigned int string_to_color(const std::string& str) {
//   std::hash<std::string> hasher;

//   unsigned int hash = hasher(str);

//   glm::vec3 rgb = hsv2rgb({hash / ((double) UINT_MAX), 1., 1.});
//   rgb *= 255;

//   unsigned int hex = ((int(rgb.r) & 0xff) << 24) + ((int(rgb.g) & 0xff) << 16) + ((int(rgb.b) & 0xff) << 8) + (255 & 0xff);

//   return RGBA_LE(hex);
// }

std::vector<legit::ProfilerTask> convert_to_profiler_task(const cevy::ecs::StageSpecs &specs, const std::list<cevy::ecs::StageTypeIndex> &indexes) {
  if (specs.map.find(std::type_index(typeid(cevy::editor::EditorPreRender))) == specs.map.end()) {
    return {};
  }

  std::vector<legit::ProfilerTask> tasks;
  auto current_stage = std::find(indexes.begin(), indexes.end(), std::type_index(typeid(cevy::editor::EditorPreRender)));
  auto last_start = specs.map.at(std::type_index(typeid(cevy::editor::EditorPreRender))).startTime;

  const float golden = 137.5f / 360.f;
  static const float hue_src = (last_start.time_since_epoch().count() & 0xfff) / double(0xfff);
  float hue = fmod(hue_src, 1);

  while (true) {
    cevy::ecs::StageTypeIndex stage_index = *current_stage;
    auto &spec = specs.map.at(stage_index);
    auto color = hsv2rgb({hue, 0.7, 0.9});

    color *= 255;
    unsigned int hex = ((int(color.r) & 0xff) << 24) | ((int(color.g) & 0xff) << 16) | ((int(color.b) & 0xff) << 8) | (255 & 0xff);
    hex = RGBA_LE(hex);

    tasks.push_back(legit::ProfilerTask {
      .startTime = (double) std::chrono::duration_cast<std::chrono::nanoseconds>(spec.startTime - last_start).count() / 1000000,
      .endTime = (double) std::chrono::duration_cast<std::chrono::nanoseconds>(spec.endTime - last_start).count() / 1000000,
      .name = stage_index.name(),
      .color = hex
    });

    hue = fmod(hue + golden, 1);

    current_stage++;
    if (current_stage == indexes.end()) {
      current_stage = indexes.cbegin();
    }
    if (*current_stage == std::type_index(typeid(cevy::editor::EditorPreRender))) {
      return tasks;
    }
  }
}

void cevy::editor::ProfilingWindow::render(cevy::editor::Editor &, glWindow &, cevy::ecs::World &world) {
  auto now = std::chrono::high_resolution_clock::now();
  auto elapsed_time = now - this->last_call;
  this->frames.push_back(
      1. /
      (std::chrono::duration_cast<std::chrono::microseconds>(elapsed_time).count() / 1000000.f));
  if (this->frames.size() > 120) {
    this->frames.erase(frames.begin());
  }
  this->framesAverage.push_back(ImGui::GetIO().Framerate);
  if (this->framesAverage.size() > 120) {
    this->framesAverage.erase(framesAverage.begin());
  }
  ImGui::Text("Elapsed time since last frame: %.3f ms",
              std::chrono::duration_cast<std::chrono::microseconds>(elapsed_time).count() / 1000.f);
  ImGui::Text("Current FPS %.3f", ImGui::GetIO().Framerate);
  ImGui::Text("Framerate");
  ImGui::PlotHistogram("##Framerate", &frames[0], frames.size(), 0, NULL, 0.0f, 140.0f,
                       ImVec2(300, 100));
  ImGui::Text("Running average framerate");
  ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(0, 235, 90, 255));
  ImGui::PlotHistogram("##Running average framerate", &framesAverage[0], frames.size(), 0, NULL,
                       0.0f, 140.0f, ImVec2(300, 100));
  ImGui::PopStyleColor();


  auto o_specs = world.get_resource<cevy::ecs::StageSpecs>();

  if (o_specs.has_value()) {
    auto &specs = o_specs->get();
    auto tasks = convert_to_profiler_task(specs, world.resource<ecs::ScheduleOrder>().order);

    legitProfiler.cpuGraph.LoadFrameData(tasks.data(), tasks.size());
    legitProfiler.gpuGraph.LoadFrameData({}, 0);
    legitProfiler.Render();
  }
  this->last_call = now;

}
