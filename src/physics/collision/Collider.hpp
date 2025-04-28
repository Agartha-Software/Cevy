/*
** EPITECH PROJECT, 2024
** R-Type
** File description:
** Collider.hpp
*/

#pragma once

#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL

#include <stdexcept>
#include <type_traits>
#include <cmath>
#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/matrix.hpp>

namespace cevy::physics {
namespace detail {
  template <typename R, typename T> R signum(T val) {
    return (T(0) < val) - (val < T(0));
  }
  inline bool solve_quadratic(float a, float b, float c, float &x0, float &x1) {
    float discr = b * b - 4 * a * c;
    if (discr < 0)
      return false;

    if (discr == 0) {
      x0 = x1 = -0.5 * b / a;
    } else {
      float q = -0.5 * (b + std::sqrt(discr) * signum<int>(b));
      x0 = q / a;
      x1 = c / q;
    }
    if (x0 > x1)
      std::swap(x0, x1);

    return true;
  }
} // namespace detail

struct Collision {
  glm::vec3 direction;
  glm::vec3 location;
  float intersection;
  bool hit;
  static constexpr Collision NoHit() { return Collision{{}, {}, 0, false};};
  Collision negate() const { return {-direction, location, intersection, hit}; }
};
struct Ray {
  glm::vec3 origin;
  glm::vec3 direction;

  template <typename T_it>
  static std::enable_if_t<std::is_same_v<typename T_it::value_type, Collision>, Collision>
  filter(const Ray &self, const T_it &begin, const T_it &end) {
    float best_dist = INFINITY;
    Collision best = Collision::NoHit();
    for (auto it_collision = begin; it_collision < end; it_collision += 1) {
      if (it_collision->hit) {
        auto dist = glm::length(it_collision->location - self.origin);
        if (dist < best_dist) {
          best_dist = dist;
          best = *it_collision;
        }
      }
    }
    return best;
  }

  template <typename T_it>
  static std::enable_if_t<std::is_same_v<decltype(std::get<Collision>(*T_it())), Collision&>,
  T_it>
  filter(const Ray &self, const T_it &begin, const T_it &end) {
    float best_dist = INFINITY;
    T_it best = end;
    for (auto it_collision = begin; it_collision < end; it_collision += 1) {
      if (std::get<Collision>(*it_collision).hit) {
        auto dist = glm::length(std::get<Collision>(*it_collision).location - self.origin);
        if (dist < best_dist) {
          best_dist = dist;
          best = it_collision;
        }
      }
    }
    return best;
  }
};

class Shape {
  public:
  enum class ShapeE {
    Sphere,
    Plane,
    // BoundedPlane,
    // Cube,
  };

  // private:
  glm::vec4 data[4];
  float dragCoefficient = 2; /// dimensionless c_d (defaulted to a cube) : ()
  float area;
  ShapeE shape;
  protected:
  Shape() {};

  public:
  static Shape Sphere(glm::vec3 position, float radius) {
    Shape shape;
    shape.shape = ShapeE::Sphere;
    shape.dragCoefficient = 0.5;
    shape.area = glm::pi<float>() * radius * radius;
    shape.data[0] = {position, 1};
    shape.data[1] = {glm::vec3(radius), 0};
    return shape;
  }

  static Shape Plane(glm::vec3 position, glm::vec3 normal) {
    Shape shape;
    shape.shape = ShapeE::Plane;
    shape.area = 1;
    shape.data[0] = {position, 1};
    shape.data[1] = {normal, 0};
    return shape;
  }

  bool isSphere() const { return this->shape == ShapeE::Sphere; }

  bool isPlane() const { return this->shape == ShapeE::Plane; }

  Collision raycast(const glm::mat4 &tm, const Ray& ray) const {
    switch (this->shape) {
    case ShapeE::Sphere:
      return Shape::raycast_sphere(this->data, tm, ray);
    case ShapeE::Plane:
      return Shape::raycast_plane(this->data, tm, ray);
    default:
    throw std::runtime_error("Shape::raycast: unreachable");
    }
    throw std::runtime_error("Shape::raycast: unreachable");
  }

  static Collision collide(const Shape &a, glm::mat4 tm_a, const Shape &b, glm::mat4 tm_b) {
    switch (a.shape) {
    case ShapeE::Sphere:
      switch (b.shape) {
      case ShapeE::Sphere:
        return Shape::collide_sphere_sphere(a.data, tm_a, b.data, tm_b);
      case ShapeE::Plane:
        return Shape::collide_sphere_plane(a.data, tm_a, b.data, tm_b);
      default:
        throw std::runtime_error("Shape::collide: unreachable");
      }
    case ShapeE::Plane:
      switch (b.shape) {
      case ShapeE::Sphere:
        return Shape::collide_sphere_plane(b.data, tm_b, a.data, tm_a);
      case ShapeE::Plane:
        return {{}, {}, 0, false};
      default:
        throw std::runtime_error("Shape::collide: unreachable");
      }
    default:
      throw std::runtime_error("Shape::collide: unreachable");
    }
    throw std::runtime_error("Shape::collide: unreachable");
  }

  float drag() const { return this->dragCoefficient; }

  private:
  static Collision raycast_sphere(const glm::vec4 (&data)[4], const glm::mat4 &tm, const Ray &ray) {
    auto i_tm = glm::inverse(tm);
    auto ray_origin = ((i_tm * glm::vec4(ray.origin, 1)) - data[0]).xyz() ;
    auto ray_direction = (i_tm * glm::vec4(ray.direction, 0)).xyz();

    glm::vec3 L = ray_origin;
    float a = glm::dot(ray_direction, ray_direction);
    float b = 2 * glm::dot(ray_direction, L);
    float c = glm::dot(L, L) - data[1].x * data[1].x;
    float t0, t1;
    if (!detail::solve_quadratic(a, b, c, t0, t1))
      return Collision::NoHit();

    glm::vec4 location;
    glm::vec3 normal;
    if (t0 > 0) {
      location = tm * glm::vec4(ray_direction * t0 + ray_origin, 1);
      normal = glm::normalize(ray_direction * t0 + ray_origin);
    } else if (t1 > 0) {
      location = tm * glm::vec4(ray_direction * t1 + ray_origin, 1);
      normal = glm::normalize(ray_direction * t1 + ray_origin);
    } else {
      return Collision::NoHit();
    }
    return Collision {normal, location, 0, true};
  }

  static Collision raycast_plane_old(const glm::vec4 (&data)[4], const glm::mat4 &tm, const Ray &ray) {
    auto i_tm = glm::inverse(tm);
    auto ray_origin = ((i_tm * glm::vec4(ray.origin, 1)) - data[0]).xyz() ;
    auto ray_direction = (i_tm * glm::vec4(ray.direction, 0)).xyz();

    auto plane_normal = (data[1]).xyz();
    // std::cout << cevy::reflect(ray_direction) << std::endl;
    // Assuming vectors are all normalized
    float denom = glm::dot(ray_direction, -plane_normal);
    // std::cout << cevy::reflect(denom) << std::endl;
    if (std::abs(denom) < 1e-6) {
      return Collision::NoHit();
    }
    // std::cout << cevy::reflect(t) << std::endl;
    float t = -glm::dot(ray_origin, -plane_normal) / std::abs(denom);


    if (t <= 0) {
      return Collision::NoHit();
    }

    glm::vec3 location = tm * glm::vec4(ray_direction * t + ray_origin, 1);
    return Collision {plane_normal * detail::signum<float>(denom), location, 0, true};
  }

  static Collision raycast_plane(const glm::vec4 (&data)[4], const glm::mat4 &tm, const Ray &ray) {

    auto plane_pos = (tm * data[0]).xyz();
    auto plane_normal = (tm * data[1]).xyz();
    // std::cout << cevy::reflect(ray_direction) << std::endl;
    // Assuming vectors are all normalized
    float denom = glm::dot(ray.direction, -plane_normal);
    // std::cout << cevy::reflect(denom) << std::endl;
    if (std::abs(denom) < 1e-6) {
      return Collision::NoHit();
    }
    // std::cout << cevy::reflect(t) << std::endl;
    float t = glm::dot(plane_pos - ray.origin, -plane_normal) / denom;


    if (t <= 0) {
      return Collision::NoHit();
    }

    glm::vec3 location = ray.direction * t + ray.origin;
    return Collision {plane_normal * detail::signum<float>(denom), location, 0, true};
  }

  static Collision collide_sphere_sphere(const glm::vec4 (&data_a)[4], const glm::mat4 &tm_a,
                                         const glm::vec4 (&data_b)[4], const glm::mat4 &tm_b) {
    auto delta = (tm_a * data_a[0] - tm_b * data_b[0]).xyz();

    auto length = glm::length(delta);
    auto intersection = ((tm_a * data_a[1]).x + (tm_b * data_b[1]).x) - length;
    if (intersection >= 0.0f) {
      auto direction = delta / length;
      // std::cout << "S/S collision : " << glm::to_string(delta) << std::endl;
      auto location = (tm_a * data_a[0]).xyz() - direction * (tm_a * data_a[1]).x;
      return {direction, location, intersection, true};
    }
    return {{}, {}, 0, false};
  }

  static Collision collide_sphere_plane(const glm::vec4 (&data_a)[4], const glm::mat4 &tm_a,
                                        const glm::vec4 (&data_b)[4], const glm::mat4 &tm_b) {
      glm::vec3 p_center = (tm_b * data_b[0]).xyz();
      glm::vec3 p_normal = (tm_b * data_b[1]).xyz();
      glm::vec3 s_center = (tm_a * data_a[0]).xyz();
      glm::vec3 s_size = (tm_a * data_a[1]).xyz();
      auto distance = dot(p_center - s_center, -p_normal);
      auto radius = std::abs(dot((tm_b * data_b[1]).xyz(), s_size));
      auto intersection = std::abs(distance) - radius;
      if (intersection <= 0) {
        auto direction = p_normal * detail::signum<float>(distance);
        auto location = s_center + direction * s_size.x;
        return {direction, location, -intersection, true};
      } else {
        return Collision::NoHit();
      }
  }
};
// enum CELL_SIZE {
//   X = 50,
//   Y = 50,
//   Z = 50,
// };
class Collider {
  private:
  public:
  std::vector<Shape> shapes;
  float dragCoefficient = 2; /// dimensionless c_d (defaulted to a cube) : ()
  float area = 1; /// projected area : m²

  Collider();
  template<typename ...S>
  Collider(S ...shapes) {
    this->dragCoefficient = (shapes.drag() + ...);
    this->shapes = {std::forward<S>(shapes)...};
  };

  Collision raycast(const glm::mat4 &tm, const Ray& ray) {
    float best_dist = INFINITY;
    Collision best = Collision::NoHit();
    for (auto shape : this->shapes) {
      auto collision = shape.raycast(tm, ray);
      if (collision.hit) {
        auto dist = glm::length(collision.location - ray.origin);
        if (dist < best_dist) {
          best_dist = dist;
          best = collision;
        }
      }
    }
    return best;
  }

  static std::vector<Collision> collide(const Collider &a, const glm::mat4 &tm_a, const Collider &b,
                                        const glm::mat4 &tm_b) {
    std::vector<Collision> accumulate;

    for (auto shape_a : a.shapes) {
      for (auto shape_b : b.shapes) {
        auto collision = Shape::collide(shape_a, tm_a, shape_b, tm_b);
        if (collision.hit) {
          accumulate.push_back(collision);
        }
      }
    }
    return accumulate;
  }

  float drag() const { return this->dragCoefficient; }
};

// using entity =
//     std::tuple<const Collider &, const cevy::engine::Position &, const cevy::engine::Rotation
//     &>;
// class Grid {
//   private:
//   SparseVector<SparseVector<SparseVector<std::vector<entity>>>> _cells;

//   public:
//   Grid() {};
//   ~Grid() {};

//   SparseVector<SparseVector<SparseVector<std::vector<entity>>>> getCells() const { return
//   _cells; }; void addEntity(const size_t x, const size_t y, const size_t z, const entity
//   &value); void setGrid(cevy::ecs::World &world); void collisionWithNeighboringEntities(const
//   entity &entity1);
// };
} // namespace cevy::physics
