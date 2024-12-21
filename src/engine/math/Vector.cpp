/*
** EPITECH PROJECT, 2023
** raytracer
** File description:
** Vector
*/

/*
#include "Vector.hpp"
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdlib>

using namespace cevy::engine;

#if 0 && (defined(__i386__) || defined(__x86_64__))
#include <immintrin.h>


float Vector::eval() const {
  __m128 v = _mm_load_ps(&x);
  v = _mm_mul_ps(v, v);
  v = _mm_hadd_ps(v, v);
  v = _mm_hadd_ps(v, v);
  return _mm_cvtss_f32(v);
}

Vector &Vector::operator+=(const Vector &rhs) {
  __m128 xmm0 = _mm_load_ps(&x);
  __m128 xmm1 = _mm_load_ps(&rhs.x);
  xmm0 = _mm_add_ps(xmm0, xmm1);
  _mm_store_ps(&x, xmm0);
  return *this;
}

Vector &Vector::operator-=(const Vector &rhs) {
  __m128 xmm0 = _mm_load_ps(&x);
  __m128 xmm1 = _mm_load_ps(&rhs.x);
  xmm0 = _mm_sub_ps(xmm0, xmm1);
  _mm_store_ps(&x, xmm0);
  return *this;
}

Vector &Vector::operator*=(float k) {
  __m128 k_v = _mm_set1_ps(k);
  __m128 this_v = _mm_load_ps(&x);
  this_v = _mm_mul_ps(this_v, k_v);
  _mm_store_ps(&x, this_v);
  return *this;
}

Vector &Vector::operator/=(float k) {
  __m128 k_v = _mm_set1_ps(1 / k);
  __m128 this_v = _mm_load_ps(&x);
  this_v = _mm_mul_ps(this_v, k_v);
  _mm_store_ps(&x, this_v);
  return *this;
}

float Vector::operator*(const Vector &rhs) const {
  __m128 xmm0 = _mm_load_ps(&x);
  __m128 xmm1 = _mm_load_ps(&rhs.x);
  xmm0 = _mm_mul_ps(xmm0, xmm1);
  xmm0 = _mm_hadd_ps(xmm0, xmm0);
  xmm0 = _mm_hadd_ps(xmm0, xmm0);
  return _mm_cvtss_f32(xmm0);
}

Vector Vector::scale(const Vector &rhs) const {
  Vector v;
  __m128 xmm0 = _mm_load_ps(&x);
  __m128 xmm1 = _mm_load_ps(&rhs.x);
  xmm0 = _mm_mul_ps(xmm0, xmm1);
  _mm_store_ps(&v.x, xmm0);
  return v;
}

#else

std::ostream &cevy::engine::operator<<(std::ostream &cout, const Vector &vec) {
  cout << "{ " << vec.x << ", " << vec.y << ", " << vec.z << " }";

  return cout;
}

float Vector::eval() const { return (x * x + y * y + z * z); }

Vector &Vector::operator+=(const Vector &rhs) {
  x += rhs.x;
  y += rhs.y;
  z += rhs.z;
  return *this;
}

Vector &Vector::operator-=(const Vector &rhs) {
  x -= rhs.x;
  y -= rhs.y;
  z -= rhs.z;
  return *this;
}

Vector &Vector::operator*=(float k) {
  x *= k;
  y *= k;
  z *= k;
  return *this;
}

Vector &Vector::operator/=(float k) {
  x /= k;
  y /= k;
  z /= k;
  return *this;
}

float Vector::operator*(const Vector &rhs) const { return x * rhs.x + y * rhs.y + z * rhs.z; }

Vector Vector::scale(const Vector &rhs) const { return Vector(x * rhs.x, y * rhs.y, z * rhs.z); }

#endif

Vector::Vector() : x(0), y(0), z(0){};

Vector::Vector(float x, float y, float z) : x(x), y(y), z(z){};

Vector &Vector::operator=(const Vector &rhs) {
  x = rhs.x;
  y = rhs.y;
  z = rhs.z;
  return *this;
}

bool Vector::operator==(const Vector &rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }

#if __cplusplus >= 202300
auto Vector::operator<=>(const Vector &rhs) const { return eval() - rhs.eval(); }
#endif

bool Vector::operator<(const Vector &rhs) const { return eval() < rhs.eval(); }

Vector Vector::operator+(const Vector &rhs) const {
  Vector v = *this;
  v += rhs;
  return v;
}

Vector Vector::operator-(const Vector &rhs) const {
  Vector v = *this;
  v -= rhs;
  return v;
}

float Vector::magnitude() const { return std::sqrt(eval()); }

Vector Vector::normalize() const {
  if (eval() < 0.0001)
    return *this;
  return *this / magnitude();
}

Vector Vector::saturate() const { return this->clamp(0, 1); }

Vector Vector::clamp(float min, float max) const {
  return Vector(std::clamp(x, min, max), std::clamp(y, min, max), std::clamp(z, min, max));
}

Vector Vector::operator*(float k) const {
  Vector v = *this;
  v *= k;
  return v;
}

Vector Vector::operator/(float k) const {
  Vector v = *this;
  v /= k;
  return v;
}

Vector Vector::operator/(const Vector &rhs) const {
  Vector v = Vector(x / rhs.x, y / rhs.y, z / rhs.z);
  return v;
}

Vector Vector::cross(const Vector &rhs) const {
  Vector p;
  p.x = (y * rhs.z) - (z * rhs.y);
  p.y = -((x * rhs.z) - (z * rhs.x));
  p.z = (x * rhs.y) - (y * rhs.x);
  return p;
}

void Vector::rotate(const Quaternion &rot) {
  Vector3 rotated = Vector3RotateByQuaternion(*this, rot);
  this->x = rotated.x;
  this->y = rotated.y;
  this->z = rotated.z;
}

void Vector::rotate(const Vector &rot) {
  float tmp = y;

  y = std::cos(rot.x) * y - std::sin(rot.x) * z;
  z = std::sin(rot.x) * tmp + std::cos(rot.x) * z;

  tmp = z;
  z = std::cos(rot.y) * z - std::sin(rot.y) * x;
  x = std::sin(rot.y) * tmp + std::cos(rot.y) * x;

  tmp = x;
  x = std::cos(rot.z) * x - std::sin(rot.z) * y;
  y = std::sin(rot.z) * tmp + std::cos(rot.z) * y;
}

void Vector::rotateR(const Vector &rot) {

  float tmp;

  tmp = x;
  x = std::cos(-rot.z) * x - std::sin(-rot.z) * y;
  y = std::sin(-rot.z) * tmp + std::cos(-rot.z) * y;

  tmp = z;
  z = std::cos(-rot.y) * z - std::sin(-rot.y) * x;
  x = std::sin(-rot.y) * tmp + std::cos(-rot.y) * x;

  tmp = y;
  y = std::cos(-rot.x) * y - std::sin(-rot.x) * z;
  z = std::sin(-rot.x) * tmp + std::cos(-rot.x) * z;
}

Vector Vector::reflect(const Vector &normal) const { return *this - normal * 2 * (*this * normal); }

Vector Vector::refract(const Vector &normal, float ior1, float ior2) const {
  // return (normal * (*this * (normal)) * (ior2) + *this + normal);
  float r = (ior1 / ior2);
  float c = -(*this * normal);
  float v = 1 - r * r * (1 - c * c);
  return ((*this * r) + (normal * (r * c - std::sqrt(v)))).normalize();
}

#if __cplusplus >= 202300

Vector Vector::random(float degree, const Vector &source) {
  Vector ret = source;

  if (degree < 0.001)
    return source;

  float alpha = (float)std::rand() / (float)RAND_MAX * M_PI * 2;
  float beta = (float)std::rand() / (float)RAND_MAX * M_PI * 2;

  ret = {std::lerp(source.x, std::cos(alpha) * std::cos(beta), degree),
         std::lerp(source.y, std::sin(alpha) * std::cos(beta), degree),
         std::lerp(source.z, std::sin(beta), degree)};

  return ret.normalize();
}

#endif

*/
