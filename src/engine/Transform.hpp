/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Transform
*/

#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_relational.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Pointer.hpp"

namespace cevy {
namespace engine {
class Transform {
  public:
  glm::quat rotation;
  glm::vec3 position;
  glm::vec3 scale;

  Transform() : rotation(glm::identity<glm::quat>()), position(0, 0, 0), scale(1, 1, 1) {}
  Transform(float x, float y, float z)
      : rotation(glm::identity<glm::quat>()), position(x, y, z), scale(1, 1, 1) {}
  Transform(const glm::vec3 &vec)
      : rotation(glm::identity<glm::quat>()), position(vec), scale(1, 1, 1) {}
  Transform(const glm::quat &quat) : rotation(quat), position(0, 0, 0), scale(1, 1, 1) {}

  glm::mat4 mat4() const {
    return glm::mat4(rotation) * glm::translate(glm::mat4(1), position) *
           glm::scale(glm::mat4(1), scale);
  }

  operator glm::mat4() const { return mat4(); }

  std::tuple<glm::vec3, glm::quat, glm::vec3> get() const {
    if (!cache_valid()) {
      if (_parent) {
        auto [p_vec, p_rot, p_scale] = _parent->get();
        _cache_vector = p_vec;
        _cache_scale = p_scale;
        _cache_quaternion = p_rot * _cache_quaternion;
        auto v = position * _cache_quaternion;
        _cache_vector += glm::vec3(v);
      } else {
        _cache_quaternion = rotation;
        _cache_vector = position * _cache_quaternion;
        _cache_scale = scale;
      }
    }
    return {_cache_vector, _cache_quaternion, _cache_scale};
  }

  Transform &operator*=(const Transform &other) {
    invalidate();
    auto [vec, rot, sca] = other.get();
    rotation = rotation * rot;
    position += vec;
    // scale += sca;
    return *this;
  }

  glm::vec3 operator*(const glm::vec3 &v) const {
    glm::vec3 w = v;
    auto [vec, rot, scale] = get();
    w = w * rot;
    w += vec;
    // w *= scale;
    return w;
  }

  Transform &rotateX(float deg) {
    invalidate();
    rotation = rotation * glm::quat(glm::vec3(deg, 0, 0));
    return *this;
  }

  Transform &rotateY(float deg) {
    invalidate();
    rotation = rotation * glm::quat(glm::vec3(0, deg, 0));
    return *this;
  }

  Transform &rotateZ(float deg) {
    invalidate();
    rotation = rotation * glm::quat(glm::vec3(0, 0, deg));
    return *this;
  }

  Transform &rotateXYZ(float x, float y, float z) {
    invalidate();
    rotation = rotation * glm::quat(glm::vec3(x, y, z));
    return *this;
  }

  Transform &rotateXYZ(const glm::vec3 &vec) {
    invalidate();
    rotation = rotation * glm::quat(vec);
    return *this;
  }

  Transform &setRotationX(float deg) {
    invalidate();
    rotation = glm::quat(glm::vec3(deg, 0, 0));
    return *this;
  }

  Transform &setRotationY(float deg) {
    invalidate();
    rotation = glm::quat(glm::vec3(0, deg, 0));
    return *this;
  }

  Transform &setRotationZ(float deg) {
    invalidate();
    rotation = glm::quat(glm::vec3(0, 0, deg));
    return *this;
  }

  Transform &setRotationXYZ(float x, float y, float z) {
    invalidate();
    rotation = glm::quat(glm::vec3(x, y, z));
    return *this;
  }

  Transform &setRotationXYZ(const glm::vec3 &vec) {
    invalidate();
    rotation = glm::quat(glm::vec3(vec.x, vec.y, vec.z));
    return *this;
  }

  Transform &translateX(float x) {
    invalidate();
    position.x += x;
    return *this;
  }

  Transform &translateY(float y) {
    invalidate();
    position.y += y;
    return *this;
  }

  Transform &translateZ(float z) {
    invalidate();
    position.z += z;
    return *this;
  }

  Transform &translateXYZ(float x, float y, float z) {
    invalidate();
    position += glm::vec3(x, y, z);
    return *this;
  }

  Transform &translateXYZ(const glm::vec3 &vec) {
    invalidate();
    position += vec;
    return *this;
  }

  Transform &setPositionX(float x) {
    invalidate();
    position.x = x;
    return *this;
  }

  Transform &setPositionY(float y) {
    invalidate();
    position.y = y;
    return *this;
  }

  Transform &setPositionZ(float z) {
    invalidate();
    position.z = z;
    return *this;
  }

  Transform &setPositionXYZ(float x, float y, float z) {
    invalidate();
    position = glm::vec3(x, y, z);
    return *this;
  }

  Transform &setPositionXYZ(const glm::vec3 &vec) {
    invalidate();
    position = vec;
    return *this;
  }

  Transform &scaleX(float deg) {
    invalidate();
    scale.x *= deg;
    return *this;
  }

  Transform &scaleY(float deg) {
    invalidate();
    scale.y *= deg;
    return *this;
  }

  Transform &scaleZ(float deg) {
    invalidate();
    scale.z *= deg;
    return *this;
  }

  Transform &scaleXYZ(float x, float y, float z) {
    invalidate();
    scale *= glm::vec3(x, y, z);
    return *this;
  }

  Transform &scaleXYZ(const glm::vec3 &vec) {
    invalidate();
    scale *= vec;
    return *this;
  }

  Transform &scaleXYZ(float deg) {
    invalidate();
    scale.x *= deg;
    scale.y *= deg;
    scale.z *= deg;
    return *this;
  }

  Transform &setScaleX(float deg) {
    invalidate();
    scale.x = deg;
    return *this;
  }

  Transform &setScaleY(float deg) {
    invalidate();
    scale.y = deg;
    return *this;
  }

  Transform &setScaleZ(float deg) {
    invalidate();
    scale.z = deg;
    return *this;
  }

  Transform &setScaleXYZ(float x, float y, float z) {
    invalidate();
    scale = glm::vec3{x, y, z};
    return *this;
  }

  Transform &setScaleXYZ(const glm::vec3 &vec) {
    invalidate();
    scale = vec;
    return *this;
  }

  Transform &setScaleXYZ(float deg) {
    invalidate();
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
    auto [vec, rot, sc] = get();
    v = v * rot;
    return v;
  }

  glm::vec3 up() const {
    glm::vec3 v{0, 1, 0};
    auto [vec, rot, sc] = get();
    v = v * rot;
    return v;
  }

  glm::vec3 right() const {
    glm::vec3 v{0, 1, 0};
    auto [vec, rot, _] = get();
    v = v * rot;
    return v;
  }

  glm::vec3 tan() const {
    glm::vec3 v{1, 0, 0};
    auto [vec, rot, _] = get();
    v = v * rot;
    return v;
  }

  glm::vec3 cotan() const {
    glm::vec3 v{0, 1, 0};
    auto [vec, rot, _] = get();
    v = v * rot;
    return v;
  }

  Transform &parent(const Transform &other) {
    invalidate();
    _parent = pointer<Transform>(other._lock, other);
    return *this;
  }

  protected:
  inline void invalidate() const { _cache_validity = false; }

  inline bool cache_valid() const {
    if (!_cache_validity) {
      return false;
    }
    if (_parent)
      return _parent->cache_valid();
    else
      return true;
  }

  pointer<Transform> _parent;
  std::shared_ptr<int> _lock = std::make_shared<int>();
  mutable glm::vec3 _cache_vector;
  mutable glm::vec3 _cache_scale;
  mutable glm::quat _cache_quaternion;
  mutable bool _cache_validity;
};
} // namespace engine
} // namespace cevy
