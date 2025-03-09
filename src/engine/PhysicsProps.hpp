/*
** EPITECH PROJECT, 2024
** C++evy
** File description:
** PhysicsProps
*/

#pragma once

namespace cevy {
namespace engine {
class PhysicsProps {
  public:
  float mass = 1;
  float decay = 0.005;
  PhysicsProps() {};
  ~PhysicsProps() {};

  PhysicsProps &setDecay(float val) {
    decay = val;
    return *this;
  }

  protected:
  private:
};
} // namespace engine
} // namespace cevy
