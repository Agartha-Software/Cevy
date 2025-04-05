/*
** AgarthaSoftware, 2025
** Cevy
** File description:
** Audio Plugin
*/

#include "Plugin.hpp"
#include "ecs.hpp"
#include "App.hpp"
#include <portaudio.h>

namespace cevy {
namespace audio {
class AudioPlugin : public cevy::ecs::Plugin {
  void build(cevy::ecs::App &app) override {
  }
};
} // namespace audio
}; // namespace cevy


