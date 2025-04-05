/*
** Agartha-Software, 2023
** C++evy
** File description:
** Commands
*/

#include "Commands.hpp"
#include "EntityCommands.hpp"

using cevy::ecs::Commands;
using cevy::ecs::EntityCommands;
#include "ecs.hpp"

void cevy::ecs::Commands::add(std::function<void(cevy::ecs::World &w)> &&f) {
  _world_access._command_queue.push(std::forward<decltype(f)>(f));
}

EntityCommands Commands::entity(const cevy::ecs::Entity &e) { return EntityCommands(*this, e); }
void cevy::ecs::Commands::despawn(Entity e) { _world_access.despawn(e); }
