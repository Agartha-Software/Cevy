/*
** EPITECH PROJECT, 2024
** rtype
** File description:
** Factory
*/

#pragma once

#include "App.hpp"
#include "Indirect.hpp"
#include "Plugin.hpp"
#include "EntityCommands.hpp"
#include "ecs.hpp"

template <typename T>
class cevy::ecs::Factory : virtual public Plugin {
  public:
  Factory() {};
  ~Factory() {};
  template <typename... Arg>
  void registerCommand(T t, void func(Query<T>, Commands)) {
    _commands[t] = func;
  }

  template <typename... Arg>
  void registerBundle(T t, Arg... arg) {
    _commands[t] = [arg...](Query<T> q, Commands cmd) {
      for (auto &e : q) {
        cmd.entity(e).insert(remove_indirect<Arg>(arg)...);
        cmd.entity(e).template remove<T>();
      }
    };
  }

  void build(cevy::ecs::App &app) override {
    // for (auto &m : _commands) {
    //   std::function<void(Query<T>, cevy::ecs::Commands)> f =
    //       std::get<std::function<void(Query<T>, cevy::ecs::Commands)>>(m);
    //   app.add_system<Schedule::PreUpdate>(std::move(f));
    // }
  }

  protected:
  std::unordered_map<T, std::function<void(Query<T>, cevy::ecs::Commands)>> _commands;

  private:
};
