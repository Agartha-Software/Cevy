/*
** EPITECH PROJECT, 2024
** rtype
** File description:
** NetworkPlugin
*/

#pragma once

#include <memory>

#include "App.hpp"
#include "NetworkCommands.hpp"
#include "Plugin.hpp"
#include "Synchroniser.hpp"
#include "network.hpp"
#include "network/network.hpp"

template <typename S, typename A, typename N = cevy::CevyNetwork>
class cevy::NetworkPlugin : ecs::Plugin {
  public:
  ~NetworkPlugin(){};

  NetworkPlugin(N &&net) : _net(std::move(net)), _sync(_net), _action(_net){};

  // template<typename S, typename A>
  // NetworkPlugin(S&& s, A&& a) {
  //     _sync = std::make_unique<S>(std::move(s));
  //     _action = std::make_unique<A>(std::move(a));
  // }

  void build(cevy::ecs::App &app) {
    app.init_resource<cevy::NetworkCommands>(cevy::NetworkCommands(_action, _sync, app));
    _sync.build(app);
    _action.build(app);
  };

  protected:
  NetworkPlugin();
  N _net;
  S _sync;
  A _action;

  private:
};
