/*
** Agartha-Software, 2023
** C++evy
** File description:
** Atmosphere
*/

#include "Atmosphere.hpp"
#include "Color.hpp"

using Atmosphere = cevy::engine::Atmosphere;
using Color = cevy::engine::Color;

Atmosphere::Atmosphere(const Color &fog, float fog_distance, const Color &ambiant)
    : ambiant(ambiant), fog(fog), fog_distance(fog_distance){};
