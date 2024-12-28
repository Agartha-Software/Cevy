/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Transform
*/

#pragma once

#include "Entity.hpp"
#include "Query.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <map>

namespace cevy {
namespace engine {

struct Parent {
  ecs::Entity entity;
};
class Transform {
  public:
  glm::vec3 position;
  glm::quat rotation;
  glm::vec3 scale;

  Transform()
      : position(0, 0, 0), rotation(glm::identity<glm::quat>()), scale(1, 1, 1),
        world_position(position), world_rotation(rotation), world_scale(scale) {}
  Transform(float x, float y, float z)
      : position(x, y, z), rotation(glm::identity<glm::quat>()), scale(1, 1, 1),
        world_position(position), world_rotation(rotation), world_scale(scale) {}

  Transform(const glm::vec3 &vec)
      : position(vec), rotation(glm::identity<glm::quat>()), scale(1, 1, 1),
        world_position(position), world_rotation(rotation), world_scale(scale) {}

  Transform(const glm::quat &quat)
      : position(0, 0, 0), rotation(quat), scale(1, 1, 1), world_position(position),
        world_rotation(rotation), world_scale(scale) {}

  Transform(const glm::vec3 &vec, const glm::quat &quat, const glm::vec3 scale)
      : position(vec), rotation(quat), scale(scale), world_position(position),
        world_rotation(rotation), world_scale(scale) {}

  glm::mat4 mat4() const {
    return glm::translate(glm::mat4(1), this->position) *
           (glm::mat4(this->rotation) * glm::scale(glm::mat4(1), this->scale));
  }

  operator glm::mat4() const { return mat4(); }

  Transform get_world() const {
    return {this->world_position, this->world_rotation, this->world_scale};
  }

  Transform &operator*=(const Transform &other) {
    this->position = other.scale * this->position;
    this->scale = this->scale * other.scale;

    this->position = other.rotation * this->position;
    this->rotation = other.rotation * this->rotation;

    this->position += other.position;
    return *this;
  }

  Transform operator*(const Transform &other) const {
    Transform tm = other;
    tm *= *this;
    return tm;
  }

  // glm::vec3 operator*(const glm::vec3 &v) const {
  //   glm::vec3 w = v;
  //   auto [vec, rot, scale] = get_world();
  //   w = w * rot;
  //   w += vec;
  //   // w *= scale;
  //   return w;
  // }

  Transform &rotateX(float deg) {
    rotation = rotation * glm::quat(glm::vec3(deg, 0, 0));
    return *this;
  }

  Transform &rotateY(float deg) {
    rotation = rotation * glm::quat(glm::vec3(0, deg, 0));
    return *this;
  }

  Transform &rotateZ(float deg) {
    rotation = rotation * glm::quat(glm::vec3(0, 0, deg));
    return *this;
  }

  Transform &rotateXYZ(float x, float y, float z) {
    rotation = rotation * glm::quat(glm::vec3(x, y, z));
    return *this;
  }

  Transform &rotateXYZ(const glm::vec3 &vec) {
    rotation = rotation * glm::quat(vec);
    return *this;
  }

  Transform &setRotationX(float deg) {
    rotation = glm::quat(glm::vec3(deg, 0, 0));
    return *this;
  }

  Transform &setRotationY(float deg) {
    rotation = glm::quat(glm::vec3(0, deg, 0));
    return *this;
  }

  Transform &setRotationZ(float deg) {
    rotation = glm::quat(glm::vec3(0, 0, deg));
    return *this;
  }

  Transform &setRotationXYZ(float x, float y, float z) {
    rotation = glm::quat(glm::vec3(x, y, z));
    return *this;
  }

  Transform &setRotationXYZ(const glm::vec3 &vec) {
    rotation = glm::quat(glm::vec3(vec.x, vec.y, vec.z));
    return *this;
  }

  Transform &translateX(float x) {
    position.x += x;
    return *this;
  }

  Transform &translateY(float y) {
    position.y += y;
    return *this;
  }

  Transform &translateZ(float z) {
    position.z += z;
    return *this;
  }

  Transform &translateXYZ(float x, float y, float z) {
    position += glm::vec3(x, y, z);
    return *this;
  }

  Transform &translateXYZ(const glm::vec3 &vec) {
    position += vec;
    return *this;
  }

  Transform &setPositionX(float x) {
    position.x = x;
    return *this;
  }

  Transform &setPositionY(float y) {
    position.y = y;
    return *this;
  }

  Transform &setPositionZ(float z) {
    position.z = z;
    return *this;
  }

  Transform &setPositionXYZ(float x, float y, float z) {
    position = glm::vec3(x, y, z);
    return *this;
  }

  Transform &setPositionXYZ(const glm::vec3 &vec) {
    position = vec;
    return *this;
  }

  Transform &scaleX(float deg) {
    scale.x *= deg;
    return *this;
  }

  Transform &scaleY(float deg) {
    scale.y *= deg;
    return *this;
  }

  Transform &scaleZ(float deg) {
    scale.z *= deg;
    return *this;
  }

  Transform &scaleXYZ(float x, float y, float z) {
    scale *= glm::vec3(x, y, z);
    return *this;
  }

  Transform &scaleXYZ(const glm::vec3 &vec) {
    scale *= vec;
    return *this;
  }

  Transform &scaleXYZ(float deg) {
    scale.x *= deg;
    scale.y *= deg;
    scale.z *= deg;
    return *this;
  }

  Transform &setScaleX(float deg) {
    scale.x = deg;
    return *this;
  }

  Transform &setScaleY(float deg) {
    scale.y = deg;
    return *this;
  }

  Transform &setScaleZ(float deg) {
    scale.z = deg;
    return *this;
  }

  Transform &setScaleXYZ(float x, float y, float z) {
    scale = glm::vec3{x, y, z};
    return *this;
  }

  Transform &setScaleXYZ(const glm::vec3 &vec) {
    scale = vec;
    return *this;
  }

  Transform &setScaleXYZ(float deg) {
    scale.x = deg;
    scale.y = deg;
    scale.z = deg;
    return *this;
  }

  glm::vec3 euler() const {
    auto v = glm::eulerAngles(rotation);
    return glm::vec3(v.x, v.y, v.z);
  }

  glm::vec3 xyz() const { return position; }

  glm::vec3 fwd() const {
    glm::vec3 v{0, 0, 1};
    v = v * this->rotation;
    return v;
  }

  glm::vec3 up() const {
    glm::vec3 v{0, 1, 0};
    v = v * this->rotation;
    return v;
  }

  glm::vec3 right() const {
    glm::vec3 v{0, 1, 0};
    v = v * this->rotation;
    return v;
  }

  glm::vec3 tan() const {
    glm::vec3 v{1, 0, 0};
    v = v * this->rotation;
    return v;
  }

  glm::vec3 cotan() const {
    glm::vec3 v{0, 1, 0};
    v = v * this->rotation;
    return v;
  }

  protected:
  template<template<typename T> typename Windower, typename Renderer>
  friend class Engine;

  Transform &parent(const Transform &parent) {
    auto world = parent.get_world();
    this->world_position = this->position;
    this->world_rotation = this->rotation;
    this->world_scale = this->scale;

    this->world_position = world.scale * this->world_position;
    this->world_scale *= world.scale;

    this->world_position = world.rotation * this->world_position;
    this->world_rotation = world.rotation * this->world_rotation;

    this->world_position += world.position;
    return *this;
  }

  void reset_world() {
    this->world_position = this->position;
    this->world_rotation = this->rotation;
    this->world_scale = this->scale;
  }

  glm::vec3 world_position;
  glm::quat world_rotation;
  glm::vec3 world_scale;

  static void parent_callback(std::map<size_t, std::tuple<Transform*, size_t>> storage, Transform& self, size_t parent) {
    if (storage.find(parent) != storage.end()) {
      auto& [p_tm, p_p] = storage.at(parent);
      parent_callback(storage, *p_tm, p_p);
      std::get<1>(storage.at(parent)) = size_t(-1);
      self.parent(*p_tm);
    }
  };

  static int children_system(ecs::Query<cevy::ecs::Entity, Parent, Transform> children, ecs::Query<ecs::Entity, Transform> all) {
    std::map<size_t, std::tuple<Transform*, size_t>> storage;
    for (auto [c_en, parent, c_tm] : children) {
      c_tm.reset_world();
      storage[c_en] = std::make_tuple(&c_tm, size_t(parent.entity));
      if (storage.find(parent.entity) == storage.end()) {
        auto q_parent = all.get(parent.entity);
        if (q_parent) {
          auto [_, p_tm] = q_parent.value();
          p_tm.reset_world();
          storage[parent.entity] = std::make_tuple(&p_tm, size_t(-1));
        }
      }
    }
    for (auto [en, s]: storage) {
      auto [tm, p] = s;
      parent_callback(storage, *tm, p);
    }
    return 0;
  }
};
} // namespace engine
} // namespace cevy
