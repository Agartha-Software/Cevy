/*
** Agartha-Software, 2023
** C++evy
** File description:
** Default Plugin
*/

#include "DefaultPlugin.hpp"
#include "App.hpp"
#include "Time.hpp"

void init_default_schedules(cevy::ecs::App &app) {
  using namespace cevy::ecs::core_stage;

  app.add_stage<Startup>();
  app.add_stage<PreStartup>();
  app.add_stage<PostStartup>();

  app.add_stage<First>();
  app.add_stage<Update>();
  app.add_stage<PreUpdate>();
  app.add_stage<PostUpdate>();
  app.add_stage<Last>();
}

void cevy::ecs::DefaultPlugin::build(App &app) {
  init_default_schedules(app);
  app.add_event<AppExit>();
  app.init_resource<StageSpecs>();
  app.add_systems<core_stage::PreStartup>(Time::init_timer);
  app.add_systems<core_stage::PostStartup>(Time::start_timer);
  app.add_systems<core_stage::First>(Time::update_timer);
}
