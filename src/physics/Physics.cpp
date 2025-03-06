/*
** EPITECH PROJECT, 2023
** R-Type
** File description:
** physics.cpp
*/

#include "Physics.hpp"
#include "App.hpp"
#include "DefaultPlugin.hpp"
#include "collision/Collider.hpp"
#include "ecs.hpp"

void check_collision(cevy::ecs::World &world) {
  cevy::physics::Grid grid;
  grid.setGrid(world);

  for (std::size_t x = 0; x < grid.getCells().size(); x++) {
    for (std::size_t y = 0; y < grid.getCells()[x].value().size(); y++) {
      for (std::size_t z = 0; z < grid.getCells()[x].value()[y].value().size(); z++) {
        if (grid.getCells()[x].value()[y].value()[z] != std::nullopt) {
          for (auto &entity : grid.getCells()[x].value()[y].value()[z].value()) {
            grid.collisionWithNeighboringEntities(entity);
          }
        }
      }
    }
  }
}

void cevy::physics::PhysicsPlugin::build(cevy::ecs::App &app) {
  app.add_plugins(cevy::ecs::DefaultPlugin());
  app.init_component<cevy::physics::Collider>();
  app.add_system<cevy::ecs::Schedule::Update>(check_collision);
}