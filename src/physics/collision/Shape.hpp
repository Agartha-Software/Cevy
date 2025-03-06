/*
** EPITECH PROJECT, 2023
** R-Type
** File description:
** Shape.hpp
*/

#include "Vector.hpp"
#include "engine/math/Vector.hpp"

namespace cevy::physics {
class Shape {
  private:
  cevy::engine::Vector _position;

  public:
  virtual ~Shape(){};
  virtual void set_position(const cevy::engine::Vector &position) = 0;
  virtual const cevy::engine::Vector &getPosition() = 0;
  virtual bool calculate_collision(Shape &other) = 0;
};
class Cuboid : public Shape {
  private:
  cevy::engine::Vector _dimension;
  cevy::engine::Vector _position;

  public:
  Cuboid(double weight, double depth, double height)
      : _dimension(weight, depth, height), _position(0, 0, 0){};

  void set_position(const cevy::engine::Vector &position) override { _position = position; }

  const cevy::engine::Vector &getPosition() override { return _position; }

  const cevy::engine::Vector &getDimension() const { return _dimension; }

  bool calculate_collision(Shape &other) override;
};

} // namespace cevy::physics