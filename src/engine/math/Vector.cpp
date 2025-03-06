/*
** EPITECH PROJECT, 2023
** raytracer
** File description:
** vector
*/

#include "Vector.hpp"
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdlib>

using namespace cevy::engine;

#if 0 && (defined(__i386__) || defined(__x86_64__))
#include <immintrin.h>


float vector::eval() const {
  __m128 v = _mm_load_ps(&x);
  v = _mm_mul_ps(v, v);
  v = _mm_hadd_ps(v, v);
  v = _mm_hadd_ps(v, v);
  return _mm_cvtss_f32(v);
}

vector &vector::operator+=(const vector &rhs) {
  __m128 xmm0 = _mm_load_ps(&x);
  __m128 xmm1 = _mm_load_ps(&rhs.x);
  xmm0 = _mm_add_ps(xmm0, xmm1);
  _mm_store_ps(&x, xmm0);
  return *this;
}

vector &vector::operator-=(const vector &rhs) {
  __m128 xmm0 = _mm_load_ps(&x);
  __m128 xmm1 = _mm_load_ps(&rhs.x);
  xmm0 = _mm_sub_ps(xmm0, xmm1);
  _mm_store_ps(&x, xmm0);
  return *this;
}

vector &vector::operator*=(float k) {
  __m128 k_v = _mm_set1_ps(k);
  __m128 this_v = _mm_load_ps(&x);
  this_v = _mm_mul_ps(this_v, k_v);
  _mm_store_ps(&x, this_v);
  return *this;
}

vector &vector::operator/=(float k) {
  __m128 k_v = _mm_set1_ps(1 / k);
  __m128 this_v = _mm_load_ps(&x);
  this_v = _mm_mul_ps(this_v, k_v);
  _mm_store_ps(&x, this_v);
  return *this;
}

float vector::operator*(const vector &rhs) const {
  __m128 xmm0 = _mm_load_ps(&x);
  __m128 xmm1 = _mm_load_ps(&rhs.x);
  xmm0 = _mm_mul_ps(xmm0, xmm1);
  xmm0 = _mm_hadd_ps(xmm0, xmm0);
  xmm0 = _mm_hadd_ps(xmm0, xmm0);
  return _mm_cvtss_f32(xmm0);
}

vector vector::scale(const vector &rhs) const {
  vector v;
  __m128 xmm0 = _mm_load_ps(&x);
  __m128 xmm1 = _mm_load_ps(&rhs.x);
  xmm0 = _mm_mul_ps(xmm0, xmm1);
  _mm_store_ps(&v.x, xmm0);
  return v;
}

#else

std::ostream &cevy::engine::operator<<(std::ostream &cout, const vector &vec) {
  cout << "{ " << vec.x << ", " << vec.y << ", " << vec.z << " }";

  return cout;
}

float vector::eval() const { return (x * x + y * y + z * z); }

vector &vector::operator+=(const vector &rhs) {
  x += rhs.x;
  y += rhs.y;
  z += rhs.z;
  return *this;
}

vector &vector::operator-=(const vector &rhs) {
  x -= rhs.x;
  y -= rhs.y;
  z -= rhs.z;
  return *this;
}

vector &vector::operator*=(float k) {
  x *= k;
  y *= k;
  z *= k;
  return *this;
}

vector &vector::operator/=(float k) {
  x /= k;
  y /= k;
  z /= k;
  return *this;
}

float vector::operator*(const vector &rhs) const { return x * rhs.x + y * rhs.y + z * rhs.z; }

vector vector::scale(const vector &rhs) const { return vector(x * rhs.x, y * rhs.y, z * rhs.z); }

#endif

vector::vector() : x(0), y(0), z(0){};

vector::vector(float x, float y, float z) : x(x), y(y), z(z){};

vector &vector::operator=(const vector &rhs) {
  x = rhs.x;
  y = rhs.y;
  z = rhs.z;
  return *this;
}

bool vector::operator==(const vector &rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }

#if __cplusplus >= 202300
auto vector::operator<=>(const vector &rhs) const { return eval() - rhs.eval(); }
#endif

bool vector::operator<(const vector &rhs) const { return eval() < rhs.eval(); }

vector vector::operator+(const vector &rhs) const {
  vector v = *this;
  v += rhs;
  return v;
}

vector vector::operator-(const vector &rhs) const {
  vector v = *this;
  v -= rhs;
  return v;
}

float vector::magnitude() const { return std::sqrt(eval()); }

vector vector::normalize() const {
  if (eval() < 0.0001)
    return *this;
  return *this / magnitude();
}

vector vector::saturate() const { return this->clamp(0, 1); }

vector vector::clamp(float min, float max) const {
  return vector(std::clamp(x, min, max), std::clamp(y, min, max), std::clamp(z, min, max));
}

vector vector::operator*(float k) const {
  vector v = *this;
  v *= k;
  return v;
}

vector vector::operator/(float k) const {
  vector v = *this;
  v /= k;
  return v;
}

vector vector::operator/(const vector &rhs) const {
  vector v = vector(x / rhs.x, y / rhs.y, z / rhs.z);
  return v;
}

vector vector::cross(const vector &rhs) const {
  vector p;
  p.x = (y * rhs.z) - (z * rhs.y);
  p.y = -((x * rhs.z) - (z * rhs.x));
  p.z = (x * rhs.y) - (y * rhs.x);
  return p;
}

void vector::rotate(const Quaternion &rot) {
  Vector3 rotated = Vector3RotateByQuaternion(*this, rot);
  this->x = rotated.x;
  this->y = rotated.y;
  this->z = rotated.z;
}

void vector::rotate(const vector &rot) {
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

void vector::rotateR(const vector &rot) {

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

vector vector::reflect(const vector &normal) const { return *this - normal * 2 * (*this * normal); }

vector vector::refract(const vector &normal, float ior1, float ior2) const {
  // return (normal * (*this * (normal)) * (ior2) + *this + normal);
  float r = (ior1 / ior2);
  float c = -(*this * normal);
  float v = 1 - r * r * (1 - c * c);
  return ((*this * r) + (normal * (r * c - std::sqrt(v)))).normalize();
}

#if __cplusplus >= 202300

vector vector::random(float degree, const vector &source) {
  vector ret = source;

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
