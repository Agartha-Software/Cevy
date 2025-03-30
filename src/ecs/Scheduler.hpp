/*
** Agartha-Software, 2023
** C++evy
** File description:
** Scheduler
*/

#pragma once

#include <functional>
#include <list>
#include <tuple>
#include <chrono>
#include <typeindex>

#include "Event.hpp"
#include "World.hpp"
#include "ecs.hpp"

namespace cevy::ecs {

struct AppExit {};

struct StageSpec {
  std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
  std::chrono::time_point<std::chrono::high_resolution_clock> endTime;
};

struct StageSpecs {
  std::unordered_map<std::type_index, StageSpec> map;
};

using StageTypeIndex = std::type_index;

struct ScheduleOrder {
  std::list<StageTypeIndex> order;
};

struct StartupScheduleOrder {
  std::list<StageTypeIndex> order;
};

class Scheduler {
  using SystemId = size_t;

  private:
  SystemId last_id = 0;

  public:
  using system_function = std::function<void(World &)>;
  using system = std::tuple<system_function, std::type_index>;
  std::vector<system> _systems;
  Scheduler() {};
  ~Scheduler() = default;

  template <class F, class S, class... Args>
  void add_class_system(const F &func) {
    static_assert(
        all(Or<is_query<Args>, is_world<Args>, is_resource<Args>, is_commands<Args>>()...),
        "type must be reference to query, world, commands or resource");

    system_function sys = [id = this->last_id, &func](World &reg) mutable { func(reg.get_super<Args>(id)...); };
    _systems.push_back(std::make_tuple(sys, std::type_index(typeid(S))));
  }

  template <class S, class R, class... Args>
  void add_system(const std::function<R(Args...)> &func) {
    static_assert(
        all(Or<is_query<Args>, is_world<Args>, is_resource<Args>, is_commands<Args>>()...),
        "type must be reference to query, world, commands or resource");

    system_function sys = [id = this->last_id, &func](World &reg) { func(reg.get_super<Args>(id)...); };
    this->last_id += 1;
    _systems.push_back(std::make_tuple(sys, std::type_index(typeid(S))));
  }

  template <class S, class R, class... Args>
  void add_system(R(func)(Args...)) {
    static_assert(
        all(Or<is_query<Args>, is_world<Args>, is_resource<Args>, is_commands<Args>,
               is_event_reader<Args>, is_event_writer<Args>>()...),
        "type must be reference to query, world, commands, event reader, event writer or resource");

    system_function sys = [id = this->last_id, func](World &reg) {
      func(reg.get_super<Args>(id)...);
    };
    last_id += 1;
    _systems.push_back(std::make_tuple(sys, std::type_index(typeid(S))));
  }

  protected:
  mutable bool _stop = false;

  void runStages(World &world, std::list<std::type_index> list);
  void runStage(World &world, std::list<std::type_index>::iterator &stage);

  private:
  /* Bevy-compliant */
  public:
  void run(World &world);
};
} // namespace cevy::ecs
