/*
** Agartha-Software, 2025
** C++evy
** File description:
** EngineBuilder
*/

#pragma once

#include "EnginePlugin.hpp"
#include "engine.hpp"
#include "glWindow.hpp"

namespace cevy {
namespace engine {

template <typename... Mod>
using EngineBuilder = Engine<glWindow::Builder<Mod...>>;

} // namespace engine
}; // namespace cevy
