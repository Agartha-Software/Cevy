/*
The Legit profiler is based on this project:
https://github.com/Raikiri/LegitProfiler
by Raikiri
It's been heavily modified to fit our needs and follows our code guidelines.
*/

#pragma once

#include "imgui.h"
#include <glm/gtx/string_cast.hpp>

namespace legit {

inline glm::vec2 to_glm_vec(const ImVec2 &vec) { return glm::vec<2, float>(vec.x, vec.y); }
inline glm::vec2 to_glm_vec(ImVec2 &&vex) { return glm::vec<2, float>(vex.x, vex.y); }
inline ImVec2 to_im_vec(const glm::vec<2, float> &vec) { return ImVec2(vec.x, vec.y); }
inline ImVec2 to_im_vec(glm::vec<2, float> &&vec) { return ImVec2(vec.x, vec.y); }

};
