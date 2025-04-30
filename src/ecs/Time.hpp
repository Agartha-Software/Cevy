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
  public:
  using duration = std::chrono::duration<double, std::ratio<1, 1>>;
  using time_point = std::chrono::time_point<std::chrono::high_resolution_clock, duration>;
  private:

  time_point _first_update;
  time_point _last_update;
  duration _last_update_delta;
  double currentTimescale = 1;
  double nextTimescale = 1;
  size_t frameCount = 0;

  public:
  duration uptime() const {
    return this->_last_update - this->_first_update;
  }

  time_point now() const {
    return this->_last_update;
  }

  void update_with_instant(time_point &&instant);

  std::chrono::duration<double, std::ratio<1>> raw() const { return this->_last_update_delta; }

  double delta_seconds() const { return this->_last_update_delta.count() * this->timescale(); }

  double timescale() const { return this->currentTimescale; }

  double setTimescale(double next) {
    auto old = this->nextTimescale;
    this->nextTimescale = next;
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
