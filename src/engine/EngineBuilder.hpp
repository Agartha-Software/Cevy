/*
** Agartha-Software, 2025
** C++evy
** File description:
** EngineBuilder
*/

#pragma once

#include "engine.hpp"
#include "glWindow.hpp"
#include "EnginePlugin.hpp"

template <typename... Mod>
using EngineBuilder = cevy::engine::Engine<glWindow::Builder<Mod...>>;
