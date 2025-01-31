/*
** Agartha-Software, 2023
** C++evy
** File description:
** Entity
*/

#pragma once

#include "ecs.hpp"

#include <cstddef>

class cevy::ecs::Entity {
  private:
  std::size_t _id;
  explicit Entity(std::size_t new_id);

  public:
  friend class ecs::World;
  friend class ecs::Commands;
  template <typename... T>
  friend class ecs::Query;
  template <typename... T>
  friend class ecs::iterator;

  bool operator==(const Entity& other) {
    return this->_id == other._id;
  }
  operator std::size_t &();
  operator std::size_t() const;
};
