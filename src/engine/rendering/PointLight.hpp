/*
** Agartha-Software, 2024
** C++evy
** File description:
** Omnidirectional point light
*/

#pragma once

#include <glm/glm.hpp>

namespace cevy::engine {
class PointLight {
public:
    glm::vec3 color;
    float radius;
};
}
