/*
** Agartha-Software, 2023
** C++evy
** File description:
** Commands
*/

#include "EntityCommands.hpp"

cevy::ecs::EntityCommands cevy::ecs::Commands::spawn_empty() {
  size_t pos = _world_access._entities.first_free(); // Should be check when multithreading
  Entity new_e = Entity(pos);

  _world_access._entities.insert_at(pos, new_e);
  return (cevy::ecs::EntityCommands(*this, new_e));
}

cevy::ecs::Entity cevy::ecs::EntityCommands::id() { return _entity; }

void cevy::ecs::EntityCommands::despawn() { _commands.get().despawn(_entity); }

cevy::ecs::Commands &cevy::ecs::EntityCommands::commands() { return _commands; }
