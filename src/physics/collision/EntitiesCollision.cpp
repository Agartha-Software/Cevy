/*
** EPITECH PROJECT, 2023
** R-Type
** File description:
** EntitiesCollision.cpp
*/

// #include "App.hpp"
#include "Collider.hpp"
#include "Physics.hpp"
#include "Position.hpp"
#include "Query.hpp"
#include "Vector.hpp"

#include <cstddef>
#include <optional>
#include <tuple>

void cevy::physics::Grid::addEntity(const size_t x, const size_t y, const size_t z,
                                    const cevy::physics::entity &value) {
  if (x >= _cells.size()) {
    _cells.resize(x + 1);
    _cells[x].value().resize(y + 1);
    _cells[x].value()[y].value().resize(z + 1);
  } else if (y >= _cells[x].value().size()) {
    _cells[x].value().resize(y + 1);
    _cells[x].value()[y].value().resize(z + 1);
  } else if (z >= _cells[x].value()[y].value().size()) {
    _cells[x].value()[y].value().resize(z + 1);
  }
  if (_cells[x].value()[y].value()[z].value().size() == 0) {
    _cells[x].value()[y].value()[z] = std::vector<cevy::physics::entity>();
  }
  _cells[x].value()[y].value()[z].value().push_back(value);
}

void cevy::physics::Grid::setGrid(ecs::World &world) {
  ecs::Query<cevy::physics::Collider, option<cevy::engine::Position>,
             option<cevy::engine::Rotation>>
      query = ecs::Query<cevy::physics::Collider, option<cevy::engine::Position>,
                         option<cevy::engine::Rotation>>::query(world);

  for (auto [collider, opt_pos, opt_rot] : query) {
    const cevy::engine::Position &position = opt_pos.value_or(cevy::engine::Position(0, 0, 0));
    const cevy::engine::Rotation &rotation = opt_rot.value_or(cevy::engine::Rotation(0, 0, 0));
    auto vec = cevy::engine::Vector(position.x, position.y, position.z);

    collider.getShape().setPosition(vec);
    size_t x = position.x / cevy::physics::CELL_SIZE::X;
    size_t y = position.y / cevy::physics::CELL_SIZE::Y;
    size_t z = position.z / cevy::physics::CELL_SIZE::Z;
    auto tuple = std::make_tuple(collider, position, rotation);
    addEntity(x, y, z, tuple);
  }
}

void cevy::physics::Grid::collisionWithNeighboringEntities(const entity &entity1) {
  for (int dx = -1; dx <= 1; dx++) {
    for (int dy = -1; dy <= 1; dy++) {
      for (int dz = -1; dz <= 1; dz++) {
        int x =
            std::get<const cevy::engine::Position &>(entity1).x / cevy::physics::CELL_SIZE::X + dx;
        int y =
            std::get<const cevy::engine::Position &>(entity1).y / cevy::physics::CELL_SIZE::Y + dy;
        int z =
            std::get<const cevy::engine::Position &>(entity1).z / cevy::physics::CELL_SIZE::Z + dz;
        if (x >= 0 && y >= 0 && z >= 0 && x < static_cast<int>(_cells.size()) &&
            y < static_cast<int>(_cells[0].value().size()) &&
            z < static_cast<int>(_cells[0].value()[0].value().size())) {
          for (auto &entity2 : _cells[x].value()[y].value()[z].value()) {
            if (std::get<const cevy::physics::Collider &>(entity1).getShape().calculateCollision(
                    std::get<const cevy::physics::Collider &>(entity2).getShape())) {
              std::cout << "entity 1 in position " << x << ", " << y << ", " << z;
              std::cout << " have a collision with the entity in position "
                        << std::get<const cevy::engine::Position &>(entity2).x << ", "
                        << std::get<const cevy::engine::Position &>(entity2).y << ", "
                        << std::get<const cevy::engine::Position &>(entity2).z << std::endl;
            }
          }
        }
      }
    }
  }
}
