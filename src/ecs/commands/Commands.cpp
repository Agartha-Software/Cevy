/*
** Agartha-Software, 2023
** C++evy
** File description:
** Commands
*/

#include "Commands.hpp"
#include "entity_commands.hpp"

using cevy::ecs::Commands;
using cevy::ecs::entity_commands;
#include "ecs.hpp"

void cevy::ecs::Commands::add(std::function<void(cevy::ecs::World &w)> &&f) {
  _world_access._command_queue.push(f);
}

entity_commands Commands::entity(const cevy::ecs::Entity &e) { return entity_commands(*this, e); }
void cevy::ecs::Commands::despawn(Entity e) { _world_access.despawn(e); }
