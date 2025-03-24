/*
** Agartha-Software, 2023
** C++evy
** File description:
** App
*/

#include "App.hpp"

void cevy::ecs::App::run() { _scheduler.run(*this); }

cevy::ecs::App::App() {
  this->init_resource<ScheduleOrder>();
  this->init_resource<StartupScheduleOrder>();
}
