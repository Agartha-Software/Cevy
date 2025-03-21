#pragma once

#include "imgui.h"
#include <glm/gtx/string_cast.hpp>

namespace legit {

inline glm::vec2 to_glm_vec(ImVec2 vec) { return glm::vec<2, float>(vec.x, vec.y); }

}
