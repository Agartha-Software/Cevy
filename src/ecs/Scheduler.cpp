/*
** Agartha-Software, 2023
** C++evy
** File description:
** Scheduler
*/

#include "Scheduler.hpp"
#include "Event.hpp"
#include "World.hpp"

using cevy::ecs::Scheduler;

void Scheduler::runStage(World &world, std::list<StageTypeIndex>::iterator &stage) {
  std::vector<std::reference_wrapper<system>> curr_sys;

  std::copy_if(_systems.begin(), _systems.end(), std::back_inserter(curr_sys),
               [&stage](const system &sys) { return std::get<1>(sys) == *stage; });

  /* this part could be multi-threaded */
  for (auto sys : curr_sys) {
    std::get<0>(sys.get())(world);
  }

  while (!world._command_queue.empty()) {
    std::function<void(World &)> func = world._command_queue.front();
    world._command_queue.pop();
    func(world);
  }
}

void Scheduler::runStages(World &world, std::list<StageTypeIndex> stage_list) {
  std::list<StageTypeIndex>::iterator stage = stage_list.begin();

  while (stage != stage_list.end()) {
    auto &stage_specs = world.resource<StageSpecs>();

    auto stageStart = std::chrono::high_resolution_clock::now();
    runStage(world, stage);
    auto stageStop = std::chrono::high_resolution_clock::now();
    stage_specs.map.insert_or_assign(*stage, StageSpec {stageStart, stageStop});
    stage++;
  }
}

void Scheduler::run(World &world) {
  runStages(world, world.resource<StartupScheduleOrder>().order);
  while (!_stop) {
    runStages(world, world.resource<ScheduleOrder>().order);
    auto close = world.get_resource<Event<AppExit>>();
    if (close && close.value().get().event_queue.size() > 0) {
      _stop = true;
    }
  }
}
