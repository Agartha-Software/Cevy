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

  public:
  std::chrono::duration<double, std::ratio<1>> startup();

  void update_with_instant(std::chrono::time_point<std::chrono::high_resolution_clock> &&instant);

  std::chrono::duration<double, std::ratio<1>> delta() const;

  double delta_seconds() const;

  Time();
};
} // namespace cevy::ecs

void init_timer(cevy::ecs::World &w);
void update_timer(cevy::ecs::Resource<cevy::ecs::Time> time);
