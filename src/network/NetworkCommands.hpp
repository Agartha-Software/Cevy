/*
** EPITECH PROJECT, 2024
** rtype
** File description:
** NetworkCommands
*/

#pragma once

#include "NetworkActions.hpp"
#include "Synchroniser.hpp"
#include "ecs.hpp"
#include "network.hpp"

/**
 * @brief Commands helper for facilitating network related actions
 *
 * binds to a given \link cevy::NetworkActions NetworkActions \endlink
 * Accessed from a system as a
 * \link cevy::ecs::Resource Resource<NetworkCommands> \endlink
 *
 */
class cevy::NetworkCommands : protected ecs::Commands {
  public:
  NetworkCommands(NetworkActions &actions, Synchroniser &sync, ecs::World &world)
      : Commands(world), _actions(actions), _sync(sync) {};
  ~NetworkCommands() {};

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
   */
  template <typename A>
  void action() {
    _actions.action<A>(*this);
  };

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
   * @param given argument to give
   */
  template <typename A>
  void action_with(typename A::Arg given) {
    _actions.event_with<A>(*this, given);
  };

  /**
   * @brief send an event
   *
   * Event will be sent, and the system will be run on the server and all clients
   *
   * @tparam E Event to send
   */
  template <typename E>
  void event() {
    _actions.event<E>(*this);
  };

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
   * @param given argument to give
   */
  template <typename E>
  void event_with(typename E::Arg given) {
    _actions.event_with<E>(*this, given);
  };

  template <typename T>
  void summon() {
    _sync.summon<T>(*this);
  };

  void dismiss(Synchroniser::SyncId id) { _sync.dismiss(*this, id); }

  protected:
  NetworkActions &_actions;
  Synchroniser &_sync;

  private:
};
