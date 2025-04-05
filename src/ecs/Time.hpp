/*
** Agartha-Software, 2023
** C++evy
** File description:
** time
*/

#pragma once

#include "ecs.hpp"
#include <chrono>
#include <ratio>

namespace cevy::ecs {
class Time {
  private:
  std::chrono::time_point<std::chrono::high_resolution_clock> _first_update;
  std::chrono::time_point<std::chrono::high_resolution_clock> _last_update;
  std::chrono::duration<double, std::ratio<1>> _last_update_delta;
  double currentTimescale = 1;
  double nextTimescale = 1;
  size_t frameCount = 0;;

  public:
  std::chrono::duration<double, std::ratio<1>> uptime() const {
    return std::chrono::high_resolution_clock::now() - _first_update;
  }

  void update_with_instant(std::chrono::time_point<std::chrono::high_resolution_clock> &&instant);

  std::chrono::duration<double, std::ratio<1>> raw() const { return _last_update_delta; }

  double delta_seconds() const { return _last_update_delta.count() * timescale(); }

  double timescale() const {
    return currentTimescale;
  }

  double setTimescale(double next) {
    auto old = nextTimescale;
    nextTimescale = next;
    return old;
  }
  size_t frame() const { return this->frameCount; }

  void reset();

  Time();
  static void init_timer(cevy::ecs::World &w);
  static void start_timer(cevy::ecs::Resource<cevy::ecs::Time> time);
  static void update_timer(cevy::ecs::Resource<cevy::ecs::Time> time);

};
} // namespace cevy::ecs
