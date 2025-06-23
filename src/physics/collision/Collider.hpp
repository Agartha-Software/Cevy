/*
** EPITECH PROJECT, 2024
** R-Type
** File description:
** Collider.hpp
*/

#pragma once

#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/matrix.hpp>

#include "engine.hpp"

namespace cevy::physics {
namespace detail {
template <typename R, typename T>
R signum(T val) {
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
  static constexpr Collision NoHit() { return Collision {{}, {}, 0, false}; };
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
  static std::enable_if_t<std::is_same_v<decltype(std::get<Collision>(*T_it())), Collision &>, T_it>
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
    Quad,
    Box,
    Cylinder,
  };

  // private:
  glm::vec4 data[4];
  std::vector<glm::vec4> bounds;
  float dragCoefficient = 2;              /// dimensionless c_d (defaulted to a cube) : ()
  float angularDragCoefficient = 2;       /// NPB defaulted to an assumed cube : ()
  float area = 1;                         /// projected area : m²
  float volume = 1;                       /// volume : m³
  glm::mat3 inertiaTensor = glm::mat3(1); /// inertia tensor : m² / rad∙s ?
  ShapeE shape;

  protected:
  Shape() {};

  public:
  static Shape Sphere(glm::vec3 position, float radius) {
    Shape shape;
    shape.shape = ShapeE::Sphere;
    shape.dragCoefficient = 0.5;
    shape.angularDragCoefficient = 0.5;
    shape.area = glm::pi<float>() * radius * radius;
    shape.volume = glm::pi<float>() * radius * radius * 4 / 3;
    shape.data[0] = {position, 1};
    shape.data[1] = {glm::vec3(radius, 0, 0), 0};
    shape.inertiaTensor = glm::mat3(2 * radius * radius / 5.f);
    return shape;
  }

  // static Shape Plane(glm::vec3 position, glm::vec3 normal) {
  //   Shape shape;
  //   shape.shape = ShapeE::Quad;
  //   shape.area = 1;
  //   shape.data[0] = {position, 1};
  //   shape.data[1] = {normal, 0};
  //   return shape;
  // }

  static Shape Quad(glm::vec3 position, glm::vec3 tangeant, glm::vec3 cotangeant) {
    glm::vec3 normal = glm::normalize(glm::cross(tangeant, cotangeant));

    Shape shape;
    shape.shape = ShapeE::Quad;
    shape.data[0] = {position, 1}; // center position
    shape.data[1] = {normal, 0};   // normal
    float tan = glm::length(tangeant);
    shape.data[2] = {tangeant / tan, tan}; // tangeant
    float cotan = glm::length(cotangeant);
    shape.data[3] = {cotangeant / cotan, cotan}; // tangeant
    shape.area = tan * cotan;
    auto height = (tan + cotan / 200.f); // 1% assumed thickness;
    shape.volume = tan * cotan * height;
    shape.inertiaTensor = {};
    shape.inertiaTensor[0][0] = (cotan * cotan + height * height) / 12.f;
    shape.inertiaTensor[1][1] = (height * height + tan * tan) / 12.f;
    shape.inertiaTensor[2][2] = (tan * tan + cotan * cotan) / 12.f;

    glm::mat3 local_to_world = glm::mat3(glm::quatLookAt(normal, cotangeant));
    glm::mat3 i_local_to_world = glm::inverse(local_to_world);

    shape.inertiaTensor = i_local_to_world * shape.inertiaTensor * glm::transpose(i_local_to_world);

    return shape;
  }

  static Shape Box(glm::vec3 position, glm::vec3 size,
                   glm::quat orientation = glm::quat({0, 0, 0})) {
    Shape shape;
    shape.shape = ShapeE::Box;
    shape.area = std::powf(size.x * size.y * size.z, 2.f / 3.f);
    shape.volume = size.x * size.y * size.z;
    shape.data[0] = {position, 1}; // center position
    shape.data[1] = {orientation * glm::vec3(size.x / 2, 0, 0), 0};
    shape.data[2] = {orientation * glm::vec3(0, size.y / 2, 0), 0};
    shape.data[3] = {orientation * glm::vec3(0, 0, size.z / 2), 0};

    glm::mat3 local_to_world = glm::mat3(orientation);
    glm::mat3 i_local_to_world = glm::inverse(local_to_world);

    shape.inertiaTensor = {};
    shape.inertiaTensor[0][0] = (size.y * size.y + size.z * size.z) / 12.f;
    shape.inertiaTensor[1][1] = (size.z * size.z + size.x * size.x) / 12.f;
    shape.inertiaTensor[2][2] = (size.x * size.x + size.y * size.y) / 12.f;

    shape.inertiaTensor = i_local_to_world * shape.inertiaTensor * glm::transpose(i_local_to_world);

    return shape;
  }

  static Shape Cylinder(glm::vec3 location, glm::quat orientation, glm::vec2 size) {
    Shape shape;
    shape.shape = ShapeE::Cylinder;
    shape.data[0] = {location, 1};
    shape.data[1] = {orientation * glm::vec3(0, 0, size.y), 0};
    shape.data[2] = {orientation * glm::vec3(size.x, 0, 0), 0};

    shape.inertiaTensor = glm::mat3 {0.1};
    return shape;
  }

  bool isSphere() const { return this->shape == ShapeE::Sphere; }

  bool isQuad() const { return this->shape == ShapeE::Quad; }

  bool isBox() const { return this->shape == ShapeE::Box; }

  Collision raycast(const glm::mat4 &tm, const Ray &ray) const {
    switch (this->shape) {
    case ShapeE::Sphere:
      return Shape::raycast_sphere(this->data, tm, ray);
    case ShapeE::Quad:
      return Shape::raycast_quad(this->data, tm, ray);
    case ShapeE::Box:
      return Shape::raycast_sphere(this->data, tm, ray);
    case ShapeE::Cylinder:
      return Collision::NoHit();
    default:
      throw std::runtime_error("Shape::raycast: unreachable");
    }
    throw std::runtime_error("Shape::raycast: unreachable");
  }

  static Collision collide(const Shape &left, glm::mat4 tm_left, const Shape &right,
                           glm::mat4 tm_right) {
    if ((int)left.shape > (int)right.shape) {
      return collide(right, tm_right, left, tm_left).negate();
    }
    const Shape &a = left;
    const Shape &b = right;
    const glm::mat4 tm_a = tm_left;
    const glm::mat4 tm_b = tm_right;

    switch (a.shape) {
    case ShapeE::Sphere:
      switch (b.shape) {
      case ShapeE::Sphere:
        return Shape::collide_sphere_sphere(a.data, tm_a, b.data, tm_b);
      case ShapeE::Quad:
        return Shape::collide_sphere_quad(a.data, tm_a, b.data, tm_b);
      case ShapeE::Box:
        return Shape::collide_sphere_box(a.data, tm_a, b.data, tm_b);
      case ShapeE::Cylinder:
        return Shape::collide_sphere_cylinder(a.data, tm_a, b.data, tm_b);
      default:
        throw std::runtime_error("Shape::collide: unreachable");
      }
    case ShapeE::Quad:
      switch (b.shape) {
      case ShapeE::Quad:
        return Collision::NoHit(); // !todo
        // return Shape::collide_quad_quad(a.data, tm_a, b.data, tm_b);
      case ShapeE::Box:
        return Shape::collide_quad_box(a.data, tm_a, b.data, tm_b);
      case ShapeE::Cylinder:
        return Shape::collide_quad_cylinder(a.data, tm_a, b.data, tm_b);
      default:
        throw std::runtime_error("Shape::collide: unreachable");
      }
    case ShapeE::Box:
      switch (b.shape) {
      case ShapeE::Box:
        return Shape::collide_box_box(a.data, tm_a, b.data, tm_b);
      case ShapeE::Cylinder:
        return Collision::NoHit(); // !todo
      // return Shape::collide_box_cylinder(a.data, tm_a, b.data, tm_b);
      default:
        throw std::runtime_error("Shape::collide: unreachable");
      }
    case ShapeE::Cylinder:
      switch (b.shape) {
      case ShapeE::Cylinder:
        // return Collision::NoHit(); // !todo
        return Shape::collide_cylinder_cylinder(a.data, tm_a, b.data, tm_b);
      default:
        throw std::runtime_error("Shape::collide: unreachable");
      }
    default:
      throw std::runtime_error("Shape::collide: unreachable");
    }
    throw std::runtime_error("Shape::collide: unreachable");
  }

  float drag() const { return this->dragCoefficient; }
  float angularDrag() const { return this->angularDragCoefficient; }

  glm::vec3 position() const { return this->data[0].xyz(); }

  private:
  static Collision raycast_sphere(const glm::vec4 (&data)[4], const glm::mat4 &tm, const Ray &ray) {
    auto i_tm = glm::inverse(tm);
    auto ray_origin = ((i_tm * glm::vec4(ray.origin, 1)) - data[0]).xyz();
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

  static Collision raycast_quad(const glm::vec4 (&data)[4], const glm::mat4 &tm, const Ray &ray) {

    auto plane_pos = (tm * data[0]).xyz();
    glm::vec3 tan = tm * glm::vec4(data[2].xyz(), 0);
    glm::vec3 cotan = tm * glm::vec4(data[3].xyz(), 0);
    auto normal = glm::cross(tan, cotan);

    float denom = glm::dot(ray.direction, -normal);

    if (std::abs(denom) < 1e-6) {
      return Collision::NoHit();
    }
    // std::cout << cevy::reflect(t) << std::endl;
    float t = glm::dot(plane_pos - ray.origin, -normal) / denom;

    if (t <= 0) {
      return Collision::NoHit();
    }

    glm::vec3 location = ray.direction * t + ray.origin;

    glm::vec3 center = tm * data[0];
    if (!(std::abs(glm::dot(location - center, tan)) < data[2].w) ||
        !(std::abs(glm::dot(location - center, cotan)) < data[3].w)) {
      return Collision::NoHit();
    }
    return Collision {normal * detail::signum<float>(denom), location, 0, true};
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

  // static Collision collide_sphere_plane(const glm::vec4 (&data_a)[4], const glm::mat4 &tm_a,
  //                                       const glm::vec4 (&data_b)[4], const glm::mat4 &tm_b) {
  //   const glm::vec3 p_center = (tm_b * data_b[0]).xyz();
  //   const glm::vec3 p_normal = (tm_b * data_b[1]).xyz();
  //   const glm::vec3 s_center = (tm_a * data_a[0]).xyz();
  //   const glm::vec3 s_size = (tm_a * data_a[1]).xyz();
  //   const auto distance = dot(p_center - s_center, -p_normal);
  //   const auto radius = std::abs(dot(glm::abs(p_normal), s_size));
  //   auto intersection = std::abs(distance) - radius;
  //   if (intersection <= 0) {
  //     auto direction = p_normal * detail::signum<float>(distance);
  //     auto location = s_center + direction * s_size.x;
  //     return {direction, location, -intersection, true};
  //   } else {
  //     return Collision::NoHit();
  //   }
  // }

  private:
  static Collision collide_sphere_edge(const glm::vec4 (&data)[4], const glm::mat4 &tm,
                                       const Ray &ray) {
    auto i_tm = glm::inverse(tm);
    auto ray_length = glm::length(ray.direction);
    auto ray_origin = ((i_tm * glm::vec4(ray.origin, 1)) - data[0]).xyz();
    auto ray_direction = (i_tm * glm::vec4(ray.direction / ray_length, 0)).xyz();

    glm::vec3 L = ray_origin;
    float a = glm::dot(ray_direction, ray_direction);
    float b = 2 * glm::dot(ray_direction, L);
    float c = glm::dot(L, L) - data[1].x * data[1].x;
    float t0, t1;
    if (!detail::solve_quadratic(a, b, c, t0, t1))
      return Collision::NoHit();

    glm::vec4 location;
    glm::vec3 normal;
    location = tm * glm::vec4(ray_direction * t0 + ray_origin, 1) +
               tm * glm::vec4(ray_direction * t1 + ray_origin, 1);
    location /= 2;
    normal = glm::normalize(i_tm * location);
    return Collision {normal, location, 0, true};
  }

  // static Collision collide_plane_edge(const glm::vec4 (&data)[4], const glm::mat4 &tm, const Ray
  // &ray) {
  //   auto ray_length = glm::length(ray.direction);
  //   auto ray_direction = ray.direction / ray_length;

  //   auto plane_pos = (tm * data[0]).xyz();
  //   auto plane_normal = (tm * data[1]).xyz();
  //   // std::cout << cevy::reflect(ray_direction) << std::endl;
  //   // Assuming vectors are all normalized
  //   float denom = glm::dot(ray_direction, -plane_normal);
  //   // std::cout << cevy::reflect(denom) << std::endl;
  //   if (std::abs(denom) < 1e-6) {
  //     return Collision::NoHit();
  //   }
  //   // std::cout << cevy::reflect(t) << std::endl;
  //   float t = glm::dot(plane_pos - ray.origin, -plane_normal) / denom;

  //   if (t <= 0 || t >= ray_length) {
  //     return Collision::NoHit();
  //   }

  //   glm::vec3 location = ray_direction * t + ray.origin;
  //   return Collision {plane_normal * detail::signum<float>(denom), location, 0, true};
  // }

  public:
  static Collision collide_sphere_quad(const glm::vec4 (&data_a)[4], const glm::mat4 &tm_a,
                                       const glm::vec4 (&data_b)[4], const glm::mat4 &tm_b) {
    const glm::vec3 s_center = (tm_a * data_a[0]).xyz();
    const glm::vec3 s_size = (tm_a * data_a[1]).xyz();

    glm::vec3 tan = tm_b * glm::vec4(data_b[2].xyz(), 0);
    float tan_w = data_b[2].w;
    glm::vec3 cotan = tm_b * glm::vec4(data_b[3].xyz(), 0);
    float cotan_w = data_b[2].w;
    const glm::vec3 q_center = (tm_b * data_b[0]).xyz();
    const glm::vec3 q_normal = glm::normalize(glm::cross(tan, cotan));
    const auto distance = dot(q_center - s_center, -q_normal);
    const auto radius = glm::length(s_size);
    // const auto radius = std::abs(dot(glm::abs(q_normal), s_size));
    auto intersection = std::abs(distance) - radius;
    if (intersection > 0) {
      // std::cout << "NoHit: !Intersection" << std::endl;
      return Collision::NoHit();
    }
    auto direction = q_normal * detail::signum<float>(distance);
    auto location = s_center + direction * radius;

    float oob_tan = glm::dot(location - q_center, tan);
    oob_tan = std::max(std::abs(oob_tan) - tan_w, 0.f) * detail::signum<float>(oob_tan);
    float oob_cotan = glm::dot(location - q_center, cotan);
    oob_cotan = std::max(std::abs(oob_cotan) - cotan_w, 0.f) * detail::signum<float>(oob_cotan);

    if (oob_cotan == 0 && oob_tan == 0) {
      return {direction, location, -intersection, true};
    } else {
      // std::cout << "NoHitYet: OOB" << std::endl;
      if (oob_tan > 0) {
        Collision pos_tan = collide_sphere_edge(
            data_a, tm_a, Ray {q_center + tan * tan_w * 0.5f - cotan * cotan_w * 0.5f, cotan});
        if (pos_tan.hit)
          return pos_tan;
      }
      if (oob_tan < 0) {
        Collision pos_tan = collide_sphere_edge(
            data_a, tm_a, Ray {q_center - tan * tan_w * 0.5f - cotan * cotan_w * 0.5f, cotan});
        if (pos_tan.hit)
          return pos_tan;
      }
      if (oob_cotan > 0) {
        Collision pos_cotan = collide_sphere_edge(
            data_a, tm_a, Ray {q_center + cotan * cotan_w * 0.5f - tan * tan_w * 0.5f, tan});
        if (pos_cotan.hit)
          return pos_cotan;
      }
      if (oob_cotan < 0) {
        Collision pos_cotan = collide_sphere_edge(
            data_a, tm_a, Ray {q_center - cotan * cotan_w * 0.5f - tan * tan_w * 0.5f, tan});
        if (pos_cotan.hit)
          return pos_cotan;
      }
    }
    return Collision::NoHit();
  }

  static Collision collide_sphere_box(const glm::vec4 (&data_a)[4], const glm::mat4 &tm_a,
                                      const glm::vec4 (&data_b)[4], const glm::mat4 &tm_b) {
    std::vector<glm::vec3> hits;

    const glm::vec3 s_center = (tm_a * data_a[0]).xyz();
    const glm::vec3 s_size = (tm_a * data_a[1]).xyz();
    auto radius = glm::length(s_size);

    const glm::vec3 b_center = (tm_b * data_b[0]).xyz();
    const glm::vec3 b_x = (tm_b * data_b[1]).xyz();
    const glm::vec3 b_y = (tm_b * data_b[2]).xyz();
    const glm::vec3 b_z = (tm_b * data_b[3]).xyz();

    {
      const auto vert = b_center + b_x + b_y + b_z;
      const auto distance = vert - s_center;

      if (glm::length(distance) < radius)
        hits.push_back(vert);
    }
    {
      const auto vert = b_center + b_x + b_y - b_z;
      const auto distance = vert - s_center;

      if (glm::length(distance) < radius)
        hits.push_back(vert);
    }
    {
      const auto vert = b_center + b_x - b_y + b_z;
      const auto distance = vert - s_center;

      if (glm::length(distance) < radius)
        hits.push_back(vert);
    }
    {
      const auto vert = b_center + b_x - b_y - b_z;
      const auto distance = vert - s_center;

      if (glm::length(distance) < radius)
        hits.push_back(vert);
    }
    {
      const auto vert = b_center - b_x + b_y + b_z;
      const auto distance = vert - s_center;

      if (glm::length(distance) < radius)
        hits.push_back(vert);
    }
    {
      const auto vert = b_center - b_x + b_y - b_z;
      const auto distance = vert - s_center;

      if (glm::length(distance) < radius)
        hits.push_back(vert);
    }
    {
      const auto vert = b_center - b_x - b_y + b_z;
      const auto distance = vert - s_center;

      if (glm::length(distance) < radius)
        hits.push_back(vert);
    }
    {
      const auto vert = b_center - b_x - b_y - b_z;
      const auto distance = vert - s_center;

      if (glm::length(distance) < radius)
        hits.push_back(vert);
    }

    if (hits.size() == 0) {
      return Collision::NoHit();
    }

    glm::vec3 hit = std::accumulate(hits.begin(), hits.end(), glm::vec3 {});
    hit /= hits.size();
    const auto normal = glm::normalize(hit - s_center);
    const auto distance = glm::dot(hit - s_center, normal);
    // const auto radius = std::abs(dot(glm::abs(normal), s_size));
    auto intersection = std::abs(distance) - radius;

    return Collision {-normal, hit, distance, true};
  }

  static Collision collide_sphere_cylinder(const glm::vec4 (&data_a)[4], const glm::mat4 &tm_a,
                                           const glm::vec4 (&data_b)[4], const glm::mat4 &tm_b) {
    const glm::vec3 s_center = (tm_a * data_a[0]).xyz();
    const glm::vec3 s_size = (tm_a * data_a[1]).xyz();
    const float s_radius = glm::length(s_size);

    const glm::vec3 c_center = (tm_b * data_b[0]).xyz();
    glm::vec4 axis = homogenous((tm_b * data_b[1]).xyz());
    const glm::vec3 c_axis = axis.xyz();
    const float c_h_height = axis.w / 2.f; // height is -1 to +1, we want 0 to 1
    const float c_radius = glm::length((tm_b * data_b[2]).xyz());

    const glm::vec3 difference = s_center - c_center;
    const float along_axis = glm::dot(c_axis, difference);
    const glm::vec3 axial_out = c_axis * along_axis;
    const glm::vec3 radial_out = difference - axial_out;
    const float along_radial = glm::length(radial_out);
    if (std::abs(along_axis) < c_h_height) {
      // center within axial bounds
      if (along_radial < c_radius) {
        // center within cylinder -> curve contact
        float intersection = along_radial - s_radius - c_radius;
        glm::vec3 normal = radial_out / along_radial;
        return Collision {-normal, s_center + normal * s_radius, intersection, true};
      } else {
        // center around curve -> curve contact
        float intersection = along_radial - s_radius - c_radius;
        if (intersection < 0) {
          glm::vec3 normal = radial_out / along_radial;
          return Collision {normal, s_center + normal * s_radius, -intersection, true};
        } else {
          return Collision::NoHit();
        }
      }
    } else {
      // center outside of axial bounds
      if (along_radial < c_radius) {
        // center facing caps -> flat contact
        glm::vec3 normal = c_axis * detail::signum<float>(along_axis);
        float intersection = std::abs(along_axis) - s_radius - c_h_height;
        if (intersection < 0) {
          return Collision {normal, s_center + normal * s_radius, -intersection, true};
        } else {
          return Collision::NoHit();
        }
      } else {
        // center on diagonal -> rim contact
        glm::vec3 corner = c_center                                                    //
                           + (c_axis * c_h_height * detail::signum<float>(along_axis)) //
                           + c_radius * radial_out / along_radial;                     //
        glm::vec3 intersecting = s_center - corner;
        float intersection = glm::length(intersecting) - s_radius;
        glm::vec3 normal = glm::normalize(intersecting);
        if (intersection < 0) {
          return Collision {normal, s_center + normal * s_radius, -intersection, true};
        } else {
          return Collision::NoHit();
        }
      }
    }
  }

  // static Collision collide_plane_quad(const glm::vec4 (&data_a)[4], const glm::mat4 &tm_a,
  //                                     const glm::vec4 (&data_b)[4], const glm::mat4 &tm_b) {
  //   std::vector<Collision> collisions;

  //   const glm::vec3 q_center = (tm_b * data_b[0]).xyz();

  //   glm::vec3 tan = tm_b * glm::vec4(data_b[2].xyz(), 0);
  //   float tan_w = data_b[2].w;
  //   glm::vec3 cotan = tm_b * glm::vec4(data_b[3].xyz(), 0);
  //   float cotan_w = data_b[2].w;

  //   {
  //     Collision pos_tan = collide_plane_edge(
  //         data_a, tm_a,
  //         Ray {q_center + tan * tan_w * 0.5f - cotan * cotan_w * 0.5f, cotan});
  //     if (pos_tan.hit)
  //       collisions.push_back(pos_tan);
  //   }
  //   {
  //     Collision pos_tan = collide_plane_edge(
  //         data_a, tm_a,
  //         Ray {q_center - tan * tan_w * 0.5f - cotan * cotan_w * 0.5f, cotan});
  //     if (pos_tan.hit)
  //         collisions.push_back(pos_tan);
  //   }
  //   {
  //     Collision pos_cotan = collide_plane_edge(
  //         data_a, tm_a,
  //         Ray {q_center + cotan * cotan_w * 0.5f - tan * tan_w * 0.5f, tan});
  //     if (pos_cotan.hit)
  //       return pos_cotan;
  //   }
  //   {
  //     Collision pos_cotan = collide_plane_edge(
  //         data_a, tm_a,
  //         Ray {q_center - cotan * cotan_w * 0.5f - tan * tan_w * 0.5f, tan});
  //     if (pos_cotan.hit)
  //       return pos_cotan;
  //   }
  // }

  //   static Collision collide_quad_quad(const glm::vec4 (&data_a)[4], const glm::mat4 &tm_a,
  //                                     const glm::vec4 (&data_b)[4], const glm::mat4 &tm_b) {
  //   std::vector<Collision> collisions;

  //   const glm::vec3 q_center = (tm_b * data_b[0]).xyz();

  //   glm::vec3 tan = tm_b * glm::vec4(data_b[2].xyz(), 0);
  //   float tan_w = data_b[2].w;
  //   glm::vec3 cotan = tm_b * glm::vec4(data_b[3].xyz(), 0);
  //   float cotan_w = data_b[2].w;

  //   {
  //     Collision pos_tan = collide_plane_edge(
  //         data_a, tm_a,
  //         Ray {q_center + tan * tan_w * 0.5f - cotan * cotan_w * 0.5f, cotan});
  //     if (pos_tan.hit)
  //       collisions.push_back(pos_tan);
  //   }
  //   {
  //     Collision pos_tan = collide_plane_edge(
  //         data_a, tm_a,
  //         Ray {q_center - tan * tan_w * 0.5f - cotan * cotan_w * 0.5f, cotan});
  //     if (pos_tan.hit)
  //         collisions.push_back(pos_tan);
  //   }
  //   {
  //     Collision pos_cotan = collide_plane_edge(
  //         data_a, tm_a,
  //         Ray {q_center + cotan * cotan_w * 0.5f - tan * tan_w * 0.5f, tan});
  //     if (pos_cotan.hit)
  //       return pos_cotan;
  //   }
  //   {
  //     Collision pos_cotan = collide_plane_edge(
  //         data_a, tm_a,
  //         Ray {q_center - cotan * cotan_w * 0.5f - tan * tan_w * 0.5f, tan});
  //     if (pos_cotan.hit)
  //       return pos_cotan;
  //   }
  // }

  static Collision collide_quad_box(const glm::vec4 (&data_a)[4], const glm::mat4 &tm_a,
                                    const glm::vec4 (&data_b)[4], const glm::mat4 &tm_b) {
    std::vector<glm::vec3> hits_below;
    std::vector<glm::vec3> hits_above;

    glm::vec3 tan = tm_a * glm::vec4(data_a[2].xyz(), 0);
    float tan_w = data_a[2].w;
    glm::vec3 cotan = tm_a * glm::vec4(data_a[3].xyz(), 0);
    float cotan_w = data_a[2].w;
    const glm::vec3 q_center = (tm_a * data_a[0]).xyz();
    const glm::vec3 q_normal = glm::normalize(glm::cross(tan, cotan));

    const glm::vec3 b_center = (tm_b * data_b[0]).xyz();
    const glm::vec3 b_x = (tm_b * data_b[1]).xyz();
    const glm::vec3 b_y = (tm_b * data_b[2]).xyz();
    const glm::vec3 b_z = (tm_b * data_b[3]).xyz();

    {
      const auto vert = b_center + b_x + b_y + b_z;
      const auto distance = dot(vert - q_center, q_normal);
      if (distance < 0)
        hits_below.push_back(vert);
      else
        hits_above.push_back(vert);
    }
    {
      const auto vert = b_center + b_x + b_y - b_z;
      const auto distance = dot(vert - q_center, q_normal);
      if (distance < 0)
        hits_below.push_back(vert);
      else
        hits_above.push_back(vert);
    }
    {
      const auto vert = b_center + b_x - b_y + b_z;
      const auto distance = dot(vert - q_center, q_normal);
      if (distance < 0)
        hits_below.push_back(vert);
      else
        hits_above.push_back(vert);
    }
    {
      const auto vert = b_center + b_x - b_y - b_z;
      const auto distance = dot(vert - q_center, q_normal);
      if (distance < 0)
        hits_below.push_back(vert);
      else
        hits_above.push_back(vert);
    }
    {
      const auto vert = b_center - b_x + b_y + b_z;
      const auto distance = dot(vert - q_center, q_normal);
      if (distance < 0)
        hits_below.push_back(vert);
      else
        hits_above.push_back(vert);
    }
    {
      const auto vert = b_center - b_x + b_y - b_z;
      const auto distance = dot(vert - q_center, q_normal);
      if (distance < 0)
        hits_below.push_back(vert);
      else
        hits_above.push_back(vert);
    }
    {
      const auto vert = b_center - b_x - b_y + b_z;
      const auto distance = dot(vert - q_center, q_normal);
      if (distance < 0)
        hits_below.push_back(vert);
      else
        hits_above.push_back(vert);
    }
    {
      const auto vert = b_center - b_x - b_y - b_z;
      const auto distance = dot(vert - q_center, q_normal);
      if (distance < 0)
        hits_below.push_back(vert);
      else
        hits_above.push_back(vert);
    }

    if (hits_below.size() == 0) {
      return Collision::NoHit();
    }
    if (hits_above.size() == 0) {
      return Collision::NoHit();
    }

    const auto &smaller = hits_above.size() < hits_below.size() ? hits_above : hits_below;

    glm::vec3 hit = std::accumulate(smaller.begin(), smaller.end(), glm::vec3 {});
    hit /= smaller.size();
    const auto distance = dot(hit - q_center, q_normal);

    return Collision {q_normal, hit, -distance, true};
  }

  static Collision collide_quad_cylinder(const glm::vec4 (&data_a)[4], const glm::mat4 &tm_a,
                                         const glm::vec4 (&data_b)[4], const glm::mat4 &tm_b) {
    glm::vec3 tan = tm_a * glm::vec4(data_a[2].xyz(), 0);
    float tan_w = data_a[2].w;
    glm::vec3 cotan = tm_a * glm::vec4(data_a[3].xyz(), 0);
    float cotan_w = data_a[2].w;
    const glm::vec3 q_center = (tm_a * data_a[0]).xyz();
    const glm::vec3 q_normal = glm::normalize(glm::cross(tan, cotan));

    const glm::vec3 c_center = (tm_b * data_b[0]).xyz();
    glm::vec4 axis = homogenous((tm_b * data_b[1]).xyz());
    const glm::vec3 c_axis = axis.xyz();
    const float c_h_height = axis.w / 2.f; // height is -1 to +1, we want 0 to 1
    const float c_radius = glm::length((tm_b * data_b[2]).xyz());

    const glm::vec3 pivot = glm::normalize(glm::cross(c_axis, q_normal));
    const glm::vec3 tilt = glm::cross(c_axis, pivot);

    auto c_a = c_center + c_axis * (c_h_height);
    auto c_b = c_center - c_axis * (c_h_height);

    std::vector<glm::vec3> points = {
        c_a + tilt * c_radius,
        c_a - tilt * c_radius,
        c_b + tilt * c_radius,
        c_b - tilt * c_radius,
    };

    const float q_dot = glm::dot(q_normal, q_center);

    float min = INFINITY;
    float max = -INFINITY;
    const glm::vec3 *v_min = 0;
    const glm::vec3 *v_max = 0;

    for (const auto &vert : points) {
      auto dot = glm::dot(q_normal, vert);
      if (dot < min) {
        min = dot;
        v_min = &vert;
      }
      if (dot > max) {
        max = dot;
        v_max = &vert;
      }
    }

    if (min > q_dot || max < q_dot) {
      return Collision::NoHit();
    }

    if (q_dot - min < max - q_dot) {
      // closest to 'min'
      return Collision {-q_normal, *v_min, std::abs(q_dot - min), true};
    } else {
      // closest to 'max'
      return Collision {q_normal, *v_max, std::abs(max - q_dot), true};
    }
  }

  protected:
  static std::tuple<float, float> compute_sat(const glm::vec3 &axis,
                                              const std::vector<glm::vec3> &verts) {
    float min = INFINITY;
    float max = -INFINITY;

    for (const auto &vert : verts) {
      auto dot = glm::dot(axis, vert);
      min = std::min(min, dot);
      max = std::max(max, dot);
    }

    return {min, max};
  }

  public:
  static Collision collide_box_box(const glm::vec4 (&data_a)[4], const glm::mat4 &tm_a,
                                   const glm::vec4 (&data_b)[4], const glm::mat4 &tm_b) {
    const glm::vec3 a_center = (tm_a * data_a[0]).xyz();
    const glm::vec3 a_x = (tm_a * data_a[1]).xyz();
    const glm::vec3 a_y = (tm_a * data_a[2]).xyz();
    const glm::vec3 a_z = (tm_a * data_a[3]).xyz();

    const glm::vec3 b_center = (tm_b * data_b[0]).xyz();
    const glm::vec3 b_x = (tm_b * data_b[1]).xyz();
    const glm::vec3 b_y = (tm_b * data_b[2]).xyz();
    const glm::vec3 b_z = (tm_b * data_b[3]).xyz();

    const glm::vec3 delta = a_center - b_center;

    std::vector<glm::vec3> verts_a = {
        a_center - a_x - a_y - a_z, //
        a_center - a_x - a_y + a_z, //
        a_center - a_x + a_y - a_z, //
        a_center - a_x + a_y + a_z, //
        a_center + a_x - a_y - a_z, //
        a_center + a_x - a_y + a_z, //
        a_center + a_x + a_y - a_z, //
        a_center + a_x + a_y + a_z, //
    };

    std::vector<glm::vec3> verts_b = {
        b_center - b_x - b_y - b_z, //
        b_center - b_x - b_y + b_z, //
        b_center - b_x + b_y - b_z, //
        b_center - b_x + b_y + b_z, //
        b_center + b_x - b_y - b_z, //
        b_center + b_x - b_y + b_z, //
        b_center + b_x + b_y - b_z, //
        b_center + b_x + b_y + b_z, //
    };

    Collision hit;
    hit.intersection = INFINITY; // used to filter to the smallest intersection
    hit.hit = true;
    hit.location = {0, 0, 0};

    for (const auto &face : {a_x, a_y, a_z, b_x, b_y, b_z}) {
      glm::vec3 axis = glm::normalize(face);
      auto [min_a, max_a] = compute_sat(axis, verts_a);
      auto [min_b, max_b] = compute_sat(axis, verts_b);

      auto top = std::max(min_a, min_b);
      auto bottom = std::min(max_a, max_b);
      auto intersection = bottom - top;
      if (max_b > min_a && max_a > min_b) {
        if (intersection < hit.intersection) {
          hit.intersection = intersection;
          hit.direction = detail::signum<float>(glm::dot(axis, delta)) * axis;
        }
        hit.location +=
            axis * (top + bottom) / 2.f /
            2.f; // add each perpendicular axis, averaged over the two bounds and over 6 axis;
      } else {
        return Collision::NoHit();
      }
    }
    return hit;
  }

  static Collision collide_cylinder_cylinder(const glm::vec4 (&data_a)[4], const glm::mat4 &tm_a,
                                             const glm::vec4 (&data_b)[4], const glm::mat4 &tm_b) {
    return Collision::NoHit();
    const glm::vec3 a_center = (tm_a * data_a[0]).xyz();
    const glm::vec3 a_run = tm_a * data_a[1];
    const glm::vec3 a_start = a_center - a_run / 2.f;
    glm::vec4 _a_axis = homogenous(a_run);
    const glm::vec3 a_axis = _a_axis.xyz();
    const float a_h_height = _a_axis.w / 2.f; // height is -1 to +1, we want 0 to 1
    glm::vec3 _rad_a = (tm_a * data_a[2]).xyz();
    const float a_radius = glm::length(_rad_a);

    const glm::vec3 b_center = (tm_b * data_b[0]).xyz();
    const glm::vec3 b_run = tm_b * data_b[1];
    const glm::vec3 b_start = b_center - b_run / 2.f;
    glm::vec4 _b_axis = homogenous(b_run);
    const glm::vec3 b_axis = _b_axis.xyz();
    const float b_h_height = _b_axis.w / 2.f; // height is -1 to +1, we want 0 to 1
    const float b_radius = glm::length((tm_b * data_b[2]).xyz());

    glm::vec3 common_n;
    glm::vec3 common_n_normalized;
    if (a_axis == b_axis) {
      return Collision::NoHit();
      //   common_n = b_center - a_center - a_axis * glm::dot(b_center - a_center, a_axis);
      // } else {
    }
    common_n = glm::cross(a_run, b_run);
    common_n_normalized = glm::normalize(common_n);

    auto diff = b_start - a_start;
    float dist = glm::dot(diff, common_n_normalized);

    if (dist - a_radius - b_radius > 0) {
      return Collision::NoHit();
    }

    float t1 = glm::dot(glm::cross(diff, b_run), common_n) / glm::dot(common_n, common_n);
    float t2 = glm::dot(glm::cross(diff, a_run), common_n) / glm::dot(common_n, common_n);

    // if (std::abs(t1) < 1 && std::abs(t2) < 1) {
    //   return Collision { common_n_normalized, a_center + a_axis * t1, -dist + a_radius +
    //   b_radius, true};
    // }

    if (t1 < 1 && t1 > 0 && t2 < 1 && t2 > 0) {
      return Collision {common_n_normalized, a_center + a_axis * t1, -dist + a_radius + b_radius,
                        true};
    }

    // if (std::abs(t1) > a_h_height + a_radius || std::abs(t2) > b_h_height + b_radius) {
    //   return Collision::NoHit();
    // }

    glm::vec3 d_a0 = b_center - b_run - a_center;

    glm::vec3 d_a1 = b_center + b_run - a_center;

    glm::vec3 d_b0 = a_center - a_run - b_center;

    glm::vec3 d_b1 = a_center + a_run - b_center;

    float intersection;
    intersection = glm::length(d_a0 - d_b0) - a_radius - b_radius;
    if (intersection < 0) {
      return Collision {glm::normalize(d_b0 - d_a0), (d_a0 + d_b0) * 0.5f, -intersection, true};
    }
    intersection = glm::length(d_a0 - d_b1) - a_radius - b_radius;
    if (intersection < 0) {
      return Collision {glm::normalize(d_b1 - d_a0), (d_a0 + d_b1) * 0.5f, -intersection, true};
    }
    intersection = glm::length(d_a1 - d_b0) - a_radius - b_radius;
    if (intersection < 0) {
      return Collision {glm::normalize(d_b0 - d_a1), (d_a1 + d_b0) * 0.5f, -intersection, true};
    }
    intersection = glm::length(d_a1 - d_b1) - a_radius - b_radius;
    if (intersection < 0) {
      return Collision {glm::normalize(d_b1 - d_a1), (d_a1 + d_b1) * 0.5f, -intersection, true};
    }
    return Collision::NoHit();

    std::vector<glm::vec3> tips = {d_a0, d_a1, d_b0, d_b1};

    d_a0 = d_a0 - glm::dot(d_a0, a_axis) * a_axis;
    d_a1 = d_a1 - glm::dot(d_a1, a_axis) * a_axis;
    d_b0 = d_b0 - glm::dot(d_b0, b_axis) * b_axis;
    d_b1 = d_b1 - glm::dot(d_b1, b_axis) * b_axis;

    std::vector<std::pair<glm::vec3, uint32_t>> deltas = {
        std::make_pair(d_a0, 0), std::make_pair(d_a1, 1), std::make_pair(d_b0, 2),
        std::make_pair(d_b1, 3)};

    auto delta_min = std::min_element(
        deltas.begin(), deltas.end(),
        [](const std::pair<glm::vec3, uint32_t> &a, const std::pair<glm::vec3, uint32_t> &b) {
          return glm::dot(a.first, a.first) < glm::dot(b.first, b.first);
        });

    // float fixed_radius = delta_min->second & 0b10 ? b_radius : a_radius;
    // float squished_radius = delta_min->second & 0b10 ? b_radius : a_radius;

    float contact_dist = glm::length(delta_min->first);
    // float intersection = contact_dist - a_radius - b_radius;
    glm::vec3 normal = delta_min->first / contact_dist;

    if (intersection < 0) {
      // return Collision::NoHit();
      return Collision {-normal,
                        tips[delta_min->second] +
                            normal * ((delta_min->second & 0b10) ? b_radius : a_radius),
                        -intersection, true};
    }
    return Collision::NoHit();
  }
};
class Collider {
  private:
  public:
  struct primitives {
    static Collider Sphere(float radius, glm::vec3 position = {0, 0, 0}) {
      return Collider(Shape::Sphere(position, radius));
    };

    static Collider Quad(glm::vec2 size, glm::quat orientation = glm::quat({0, 0, 0}),
                         glm::vec3 position = {0, 0, 0}) {
      return Collider(Shape::Quad(position, orientation * glm::vec3(size.x, 0, 0),
                                  orientation * glm::vec3(0, size.y, 0)));
    };
    // static Collider Cube(glm::vec3 sides = {1, 1, 1}, glm::vec3 position = {0, 0, 0},
    //                      glm::quat orientation = glm::quat({0, 0, 0})) {
    //   glm::vec3 x = orientation * glm::vec3(sides.x, 0, 0);
    //   glm::vec3 y = orientation * glm::vec3(0, sides.y, 0);
    //   glm::vec3 z = orientation * glm::vec3(0, 0, sides.z);
    //   return Collider(Shape::Quad(position + x / 2.f, y, z), Shape::Quad(position - x / 2.f, -y,
    //   z),

    //                   Shape::Quad(position + y / 2.f, z, y), Shape::Quad(position - y / 2.f, -z,
    //                   y),

    //                   Shape::Quad(position + z / 2.f, y, x),
    //                   Shape::Quad(position + z / 2.f, -y, x));
    // };
    static Collider Box(glm::vec3 size, glm::quat orientation = glm::quat({0, 0, 0}),
                        glm::vec3 position = {0, 0, 0}) {
      return Collider(Shape::Box(position, size, orientation));
    };

    static Collider Cylinder(glm::vec2 size, glm::quat orientation = glm::quat({0, 0, 0}),
                             glm::vec3 position = {0, 0, 0}) {
      return Collider(Shape::Cylinder(position, orientation, size));
    }
  };

  std::vector<Shape> shapes;
  uint32_t layers = -1;
  float dragCoefficient = 2;        /// dimensionless c_d (defaulted to a cube) : ()
  float angularDragCoefficient = 2; /// dimensionless NPB (defaulted to an assumed cube) : ()
  float area = 1;                   /// projected area : m²
  float volume;                     /// volume : m³

  /// inverse of inertia tensor : rad∙s / m² ?
  glm::mat4 iInertiaTensor = {1};

  Collider();
  template <typename... S>
  Collider(uint32_t layers, S &&...shapes) : Collider(std::forward<S>(shapes)...) {
    this->layers = layers;
  };
  template <typename... S, class = std::enable_if_t<std::conjunction_v<std::is_same<S, Shape>...>>>
  Collider(S... shapes) {
    this->area = (shapes.area + ...);
    this->volume = (shapes.volume + ...);
    this->dragCoefficient = (shapes.drag() + ...);
    this->angularDragCoefficient = (shapes.angularDrag() + ...);
    this->angularDragCoefficient +=
        ((shapes.drag() * shapes.area * glm::length(shapes.position())) + ...);
    this->shapes = {std::forward<S>(shapes)...};
    this->iInertiaTensor = 1.f / ((shapes.inertiaTensor) + ...);
  };

  Collision raycast(const glm::mat4 &tm, const Ray &ray) {
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
  float angularDrag() const { return this->angularDragCoefficient; }
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
