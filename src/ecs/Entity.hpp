/*
** Agartha-Software, 2023
** C++evy
** File description:
** entity
*/

#pragma once

#include "ecs.hpp"

#include <cstddef>

class cevy::ecs::entity {
  private:
  std::size_t _id;
  explicit entity(std::size_t new_id);

  public:
  friend class ecs::World;
  friend class ecs::Commands;
  template <typename... T>
  friend class ecs::Query;
  operator std::size_t &();
  operator std::size_t() const;
};
