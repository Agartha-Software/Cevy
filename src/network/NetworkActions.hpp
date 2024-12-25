/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** NetworkAction
*/

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <list>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "App.hpp"
#include "Commands.hpp"
#include "Plugin.hpp"
#include "Query.hpp"
#include "CevyNetwork.hpp"
#include "network.hpp"

/**
 * @brief A handler for network activity
 *
 * Stores and exposes systems mapped to events and actions
 * Controlled through \link cevy::NetworkCommands NetworkCommands \endlink
 *
 */
class cevy::NetworkActions : public ecs::Plugin {
  public:
  /**
   * Whether it acts on behalf of a server or client
   */
  enum class Mode {
    Server,
    Client,
  };

  /// Contructor
  NetworkActions(CevyNetwork &net) : _net(net){};

  /// deleted copy-constructor
  NetworkActions(const NetworkActions &) = delete;

  NetworkActions &operator=(const NetworkActions &) = delete;
  NetworkActions &operator=(NetworkActions &&) = delete;
  ~NetworkActions(){};

  using ActionFailureMode = CevyNetwork::ActionFailureMode;
  using EActionFailureMode = ActionFailureMode::EActionFailureMode;

  /// Presume will let an action fail or succeed before the server's explicit response
  enum class Presume {
    idk,
    fail,
    success,
  };

  /**
   *  @brief Action
   *
   * Actions are defined by their key, and can require an additional argument
   */
  template <size_t N = 0, Presume p = Presume::idk, typename Data = std::tuple<>>
  struct Action {
    inline static const uint16_t value = N;
    inline static const Presume presume = p;
    inline static const size_t serialized_size = sum(::cevy::serialized_size<Data>::value);
    using Arg = Data;
  };

  /**
   *  @brief Event
   *
   * Events are defined by their key, and can require an additional argument
   */
  template <size_t N = 0, typename Data = std::tuple<>>
  struct Event {
    inline static const uint16_t value = N;
    inline static const size_t serialized_size = sum(::cevy::serialized_size<Data>::value);
    using Arg = Data;
  };

  virtual void build(cevy::ecs::App &) override {}

  /**
   * @brief add an action with regular functions
   *
   * @tparam A Action to map to
   * @tparam Arg0 arguments to the server system
   * @tparam Arg1 arguments to the client_success system
   * @tparam Arg2 arguments to the client_fail system
   * @param server server-side system
   * @param client_success system triggered on action success
   * @param client_fail system triggered on action failure
   */
  template <typename A, typename... Arg0, typename... Arg1, typename... Arg2>
  void add_action(EActionFailureMode (&&server)(Arg0...), bool (&&client_success)(Arg1...),
                  bool (&&client_fail)(Arg2...)) {
    _actions[A::value] = std::make_tuple(
        server_system([server](ecs::Commands &cmd) mutable { return cmd.system(server); }),
        system([client_success](ecs::Commands &cmd) mutable { return cmd.system(client_success); }),
        system([client_fail](ecs::Commands &cmd) mutable { return cmd.system(client_fail); }));
  };

  /**
   * @brief add an action with std::function
   *
   * @tparam A Action to map to
   * @tparam Arg0 arguments to the server system
   * @tparam Arg1 arguments to the client_success system
   * @tparam Arg2 arguments to the client_fail system
   * @param server server-side system
   * @param client_success system triggered on action success
   * @param client_fail system triggered on action failure
   */
  template <typename A, typename... Arg0, typename... Arg1, typename... Arg2>
  void add_action(
      std::function<EActionFailureMode(Arg0...)> server,
      std::function<bool(Arg1...)> client_success = []() {},
      std::function<bool(Arg2...)> client_fail = []() {}) {
    _actions[A::value] = std::make_tuple(
        server_system([server](ecs::Commands &cmd) mutable { return cmd.system(server); }),
        system([client_success](ecs::Commands &cmd) mutable { return cmd.system(client_success); }),
        system([client_fail](ecs::Commands &cmd) mutable { return cmd.system(client_fail); }));
  }

  /**
   * @brief add an action with std::functions, taking an extra parameter
   *
   * @tparam A Action to map to
   * @tparam A::Arg extra argument, specified by the Action
   * @tparam Arg0 arguments to the server system
   * @tparam Arg1 arguments to the client_success system
   * @tparam Arg2 arguments to the client_fail system
   * @param server server-side system
   * @param client_success system triggered on action success
   * @param client_fail system triggered on action failure
   */
  template <typename A, typename... Arg0, typename... Arg1, typename... Arg2>
  void add_action_with(
      std::function<EActionFailureMode(typename A::Arg, Arg0...)> server,
      std::function<bool(typename A::Arg, Arg1...)> client_success = []() {},
      std::function<bool(typename A::Arg, Arg2...)> client_fail = []() {}) {
    using server_ftype = std::function<EActionFailureMode(ecs::Commands &, typename A::Arg)>;
    using ftype = std::function<void(ecs::Commands &, typename A::Arg)>;
    _super_actions[std::type_index(typeid(A))] = std::make_tuple(
        std::make_any<server_ftype>([server](ecs::Commands &cmd, typename A::Arg given) mutable {
          return cmd.system_with(server, given);
        }),
        std::make_any<ftype>([client_success](ecs::Commands &cmd, typename A::Arg given) mutable {
          return cmd.system_with(client_success, given);
        }),
        std::make_any<ftype>([client_fail](ecs::Commands &cmd, typename A::Arg given) mutable {
          return cmd.system_with(client_fail, given);
        }));
  }

  /**
   * @brief add an action with regular functions, taking an extra parameter
   *
   * @tparam A Action to map to
   * @tparam A::Arg extra argument, specified by the Action
   * @tparam Arg0 arguments to the server system
   * @tparam Arg1 arguments to the client_success system
   * @tparam Arg2 arguments to the client_fail system
   * @param server server-side system
   * @param client_success system triggered on action success
   * @param client_fail system triggered on action failure
   */
  template <typename A, typename... Arg0, typename... Arg1, typename... Arg2>
  void add_action_with(
      EActionFailureMode (&&server)(typename A::Arg, Arg0...),
      bool (&&client_success)(typename A::Arg, Arg1...) = []() {},
      bool (&&client_fail)(typename A::Arg, Arg2...) = []() {}) {
    using server_ftype = std::function<EActionFailureMode(ecs::Commands &, typename A::Arg)>;
    using ftype = std::function<void(ecs::Commands &, typename A::Arg)>;
    _super_actions[std::type_index(typeid(A))] = std::make_tuple(
        std::make_any<server_ftype>([server](ecs::Commands &cmd, typename A::Arg given) mutable {
          return cmd.system_with(server, given);
        }),
        std::make_any<ftype>([client_success](ecs::Commands &cmd, typename A::Arg given) mutable {
          return cmd.system_with(client_success, given);
        }),
        std::make_any<ftype>([client_fail](ecs::Commands &cmd, typename A::Arg given) mutable {
          return cmd.system_with(client_fail, given);
        }));
  }

  /**
   * @brief add an event with a regular function
   *
   * @tparam E Event to map to
   * @tparam Args arguments to the server system
   * @param func system to trigger
   */
  template <typename E, typename... Args>
  void add_event(bool (&&func)(Args...)) {
    _events[E::value] = [func](ecs::Commands &cmd) { cmd.system(func); };
  };

  /**
   * @brief add an event with a regular function, taking an extra parameter
   *
   * @tparam E Event to map to
   * @tparam E::Arg extra argument, specified by the Event
   * @tparam Args arguments to the system
   * @param func system to trigger
   */
  template <typename E, typename... Args>
  void add_event_with(bool (&&func)(typename E::Arg, Args...)) {
    auto lambda = [func](ecs::Commands &cmd, typename E::Arg given) {
      return cmd.system_with<typename E::Arg>(func, given);
    };
    _super_events[std::type_index(typeid(E))] =
        std::make_any<std::function<void(ecs::Commands &, typename E::Arg)>>(lambda);
  };

  /**
   * @brief add an event with an std::function, taking an extra parameter
   *
   * @tparam E Event to map to
   * @tparam E::Arg extra argument, specified by the Event
   * @tparam Args arguments to the system
   * @param func system to trigger
   */
  template <typename E, typename... Args>
  void add_event_with(std::function<bool(typename E::Arg, Args...)> func) {
    auto lambda = [func](ecs::Commands &cmd, typename E::Arg given) {
      return cmd.system_with<typename E::Arg>(func, given);
    };
    _super_events[std::type_index(typeid(E))] =
        std::make_any<std::function<bool(ecs::Commands &, typename E::Arg)>>(lambda);
  };

  /**
   * @brief Call an action
   *
   * when run client-side, this will send the action to the server, and preform
   * the presumed system if specified
   *
   * when run server-side, this will perform the server system following which
   * the action is responded to with a failure or success
   *
   * @tparam A Action to call
   * @param cmd a reference to Commands
   */
  template <typename A>
  void action(ecs::Commands &cmd) {
    if (_mode == Mode::Server) {
      EActionFailureMode ret = std::get<0>(_actions[A::value])(cmd);
      if (ret == ActionFailureMode::Action_Success) {
        if (A::presume != Presume::success)
          _net.sendActionSuccess(A::value, std::vector<uint8_t>({0}));
      } else {
        if (A::presume != Presume::fail)
          _net.sendActionFailure(A::value, ret);
      }
    } else {
      if (A::presume == Presume::success)
        std::get<1>(_actions[A::value])(cmd);
      else if (A::presume == Presume::fail)
        std::get<2>(_actions[A::value])(cmd);
      _net.sendAction(A::value, std::vector<uint8_t>());
    }
  }

  /**
   * @brief Call an action with a given argument
   *
   * the argument must correspond to the one specified by the Action
   *
   * when run client-side, this will send the action to the server, and preform
   * the presumed system if specified
   *
   * when run server-side, this will perform the server system following which
   * the action is responded to with a failure or success
   *
   * the argument will be send, so it must be defined for
   * \link cevy::serialize serialize \endlink
   *
   * @tparam A Action to call
   * @tparam A::Arg Argument required
   * @param cmd a reference to Commands
   * @param given argument to give
   */
  template <typename A>
  void action_with(ecs::Commands &cmd, typename A::Arg given) {
    auto &server = std::get<0>(_super_actions[std::type_index(typeid(A))]);
    auto &client_success = std::get<1>(_super_actions[std::type_index(typeid(A))]);
    auto &client_fail = std::get<2>(_super_actions[std::type_index(typeid(A))]);
    if (_mode == Mode::Server) {
      const auto &func =
          std::any_cast<std::function<EActionFailureMode(ecs::Commands &, typename A::Arg)>>(
              server);
      EActionFailureMode ret = func(cmd, given);
      if (ret == ActionFailureMode::Action_Success) {
        if (A::Presume != Presume::success)
          _net.sendActionSuccess(A::value, std::vector<uint8_t>({0}));
      } else {
        if (A::Presume != Presume::fail)
          _net.sendActionFailure(A::value, ret);
      }
    } else {
      if (A::presume == Presume::success) {
        const auto &func =
            std::any_cast<std::function<bool(ecs::Commands &, typename A::Arg)>>(client_success);
        func(cmd, given);
      } else if (A::presume == Presume::fail) {
        const auto &func =
            std::any_cast<std::function<bool(ecs::Commands &, typename A::Arg)>>(client_fail);
        func(cmd, given);
      }
      std::vector<uint8_t> vec;
      serialize(vec, given);
      if (vec.size() != serialized_size<typename A::Arg>::value)
        throw std::exception();
      _net.sendAction(A::value, vec);
    }
  }

  /**
   * @brief send an event
   *
   * Event will be sent, and the system will be run on the server and all clients
   *
   * @tparam E Event to send
   * @param cmd a reference to Commands
   */
  template <typename E>
  void event(ecs::Commands &cmd) {
    _events[E::value](cmd);
    std::vector<uint8_t> vec(E::serialized_size, 0);
    _net.sendEvent(E::value, vec);
  }

  /**
   * @brief Send an event with a given argument
   *
   * the argument must correspond to the one specified by the Event
   *
   * Event will be sent, and the system will be run on the server and all clients
   *
   * the argument will be send, so it must be defined for
   * \link cevy::serialize serialize \endlink
   *
   * @tparam E Event to send
   * @tparam E::Arg Argument required
   * @param cmd a reference to Commands
   * @param given argument to give
   */
  template <typename E>
  void event_with(ecs::Commands &cmd, typename E::Arg given) {
    const auto &func = std::any_cast<std::function<bool(ecs::Commands &, typename E::Arg)>>(
        _super_events[std::type_index(typeid(E))]);
    func(cmd, given);
    std::vector<uint8_t> vec;
    serialize(vec, given);
    if (vec.size() != serialized_size<typename E::Arg>::value)
      throw std::exception();
    _net.sendEvent(E::value, vec);
  }

  protected:
  using server_system = std::function<EActionFailureMode(ecs::Commands &)>;
  using system = std::function<bool(ecs::Commands &)>;
  std::unordered_map<uint16_t, std::tuple<server_system, system, system>> _actions;
  std::unordered_map<std::type_index, std::tuple<std::any, std::any, std::any>> _super_actions;
  std::unordered_map<uint16_t, system> _events;
  std::unordered_map<std::type_index, std::any> _super_events;
  Mode _mode;
  cevy::CevyNetwork &_net;
};
