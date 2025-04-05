/*
** Agartha-Software, 2023
** C++evy
** File description:
** time
*/

#include "Time.hpp"
#include "World.hpp"

using cevy::ecs::Time;

Time::Time() : _first_update(std::chrono::high_resolution_clock::now()) {}

void init_timer(cevy::ecs::World &w) { w.insert_resource<cevy::ecs::Time>(cevy::ecs::Time()); }

void update_timer(cevy::ecs::Resource<Time> time) {
  time.get().update_with_instant(std::chrono::high_resolution_clock::now());
}

std::chrono::duration<double, std::ratio<1>> Time::startup() {
  return std::chrono::high_resolution_clock::now() - _first_update;
}

void Time::update_with_instant(
    std::chrono::time_point<std::chrono::high_resolution_clock> &&instant) {
  _last_update_delta = instant - _last_update;
  _last_update = instant;
}

std::chrono::duration<double, std::ratio<1>> Time::delta() const { return _last_update_delta; }

double Time::delta_seconds() const { return _last_update_delta.count(); }
