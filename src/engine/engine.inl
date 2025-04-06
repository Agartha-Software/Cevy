/*
** Agartha-Software, 2023
** C++evy
** File description:
** Engine template specializations
*/

#define GLM_ENABLE_EXPERIMENTAL

#include "engine.hpp"
#include <glm/gtx/string_cast.hpp>

namespace cevy {

template <>
std::string reflect<glm::vec3>(const glm::vec3 &i) {
    return reflect<glm::vec3>() + "::" + glm::to_string(i);
}
template <>
std::string reflect<glm::vec4>(const glm::vec4 &i) {
    return reflect<glm::vec4>() + "::" + glm::to_string(i);
}
template <>
std::string reflect<glm::quat>(const glm::quat &i) {
    return reflect<glm::vec4>() + "::" + glm::to_string(i);
}
template <>
std::string reflect<glm::mat4>(const glm::mat4 &i) {
    return reflect<glm::mat4>() + "::" + glm::to_string(i);
}
} // namespace cevy
