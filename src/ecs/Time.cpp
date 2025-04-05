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

void Time::init_timer(cevy::ecs::World &w) {
  Time time;
  time.currentTimescale = 0;
  time.nextTimescale = 0;
  w.insert_resource<cevy::ecs::Time>(time);
}

void Time::start_timer(cevy::ecs::Resource<Time> time) {
  if (time->nextTimescale == 0) {
    time->setTimescale(1);
  }
}

void Time::update_timer(cevy::ecs::Resource<Time> time) {
  time->update_with_instant(std::chrono::high_resolution_clock::now());
  time->currentTimescale = time->nextTimescale;
  time->frameCount += 1;
}

void Time::update_with_instant(
    std::chrono::time_point<std::chrono::high_resolution_clock> &&instant) {
  _last_update_delta = instant - _last_update;
  _last_update = instant;
}

void Time::reset() {
  frameCount = 0;
  auto now = std::chrono::high_resolution_clock::now();
  this->_first_update = now;
  this->_last_update = now;
}
