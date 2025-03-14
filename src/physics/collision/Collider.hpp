/*
** EPITECH PROJECT, 2024
** R-Type
** File description:
** Collider.hpp
*/

#pragma once

#include <algorithm>
#include <memory>

#include "Position.hpp"
#include "Rotation.hpp"
#include "SparseVector.hpp"
#include "World.hpp"

#include "Shape.hpp"
#include "ecs.hpp"
#include <tuple>
#include <utility>
#include <vector>

namespace cevy::physics {

enum CELL_SIZE {
  X = 50,
  Y = 50,
  Z = 50,
};
class Collider {
  private:
  std::unique_ptr<Shape> _shape;

  public:
  template <typename GivenShape>
  Collider(std::unique_ptr<GivenShape> shape) : _shape(std::move(shape)) {
    static_assert(std::is_base_of<Shape, GivenShape>::value, "GivenShape must derive from Shape");
  }
  ~Collider() = default;

  Shape &getShape() const { return *_shape; }
};

using entity =
    std::tuple<const Collider &, const cevy::engine::Position &, const cevy::engine::Rotation &>;
class Grid {
  private:
  SparseVector<SparseVector<SparseVector<std::vector<entity>>>> _cells;

  public:
  Grid() {};
  ~Grid() {};

  SparseVector<SparseVector<SparseVector<std::vector<entity>>>> getCells() const { return _cells; };
  void addEntity(const size_t x, const size_t y, const size_t z, const entity &value);
  void setGrid(cevy::ecs::World &world);
  void collisionWithNeighboringEntities(const entity &entity1);
};
} // namespace cevy::physics