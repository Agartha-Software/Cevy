/*
** Agartha-Software, 2023
** rtype
** File description:
** network
*/

#pragma once

#include "cevy.hpp"
#include <bit>
#include <cstdint>
#include <exception>
#include <tuple>
#include <type_traits>
#include <vector>

namespace cevy {

template <typename T>
struct serialized_size : public std::integral_constant<size_t, 0> {};

template <typename T>
constexpr std::vector<uint8_t> &serialize(std::vector<uint8_t> &, const T &);

template <typename T>
T deserialize(std::vector<uint8_t> &);

template <template <typename...> typename T, typename... Ts>
inline constexpr std::vector<uint8_t> &serialize(std::vector<uint8_t> &vec, const T<Ts...> &t) {
  (vec.push_back(std::get<Ts>(t)), ...);
  return vec;
}

template <typename S, typename A, typename N>
class NetworkPlugin;
class Synchroniser;
class NetworkActions;
class CevyNetwork;
class NetworkBase;
class NetworkCommands;
class ClientHandler;
class ServerHandler;
using Network = CevyNetwork;
template <typename S, typename A, typename N>
class NetworkPlugin;
} // namespace cevy

template <typename... T>
struct cevy::serialized_size<std::tuple<T...>>
    : public std::integral_constant<size_t, sum(serialized_size<T>::value...)> {};

template <>
struct cevy::serialized_size<uint8_t> : public std::integral_constant<size_t, sizeof(uint8_t)> {};

template <>
inline std::vector<uint8_t> &cevy::serialize<uint8_t>(std::vector<uint8_t> &vec, const uint8_t &t) {
  vec.push_back(t);
  return vec;
}

template <>
inline uint8_t cevy::deserialize<uint8_t>(std::vector<uint8_t> &vec) {
  uint8_t t = vec.back();
  vec.pop_back();
  return t;
}

template <>
struct cevy::serialized_size<uint16_t> : public std::integral_constant<size_t, sizeof(uint16_t)> {};

template <>
inline std::vector<uint8_t> &cevy::serialize<uint16_t>(std::vector<uint8_t> &vec,
                                                       const uint16_t &t) {
  auto p = reinterpret_cast<const uint8_t *>(&t);
  vec.push_back(p[0]);
  vec.push_back(p[1]);
  return vec;
}

template <>
inline uint16_t cevy::deserialize<uint16_t>(std::vector<uint8_t> &vec) {
  uint16_t t;
  auto p = reinterpret_cast<uint8_t *>(&t);
  for (size_t i = serialized_size<uint16_t>::value; i--;) {
    p[i] = vec.back();
    vec.pop_back();
  }
  return t;
}

template <>
struct cevy::serialized_size<int> : public std::integral_constant<size_t, sizeof(int)> {};

template <>
inline std::vector<uint8_t> &cevy::serialize<int>(std::vector<uint8_t> &vec, const int &t) {
  auto p = reinterpret_cast<const uint8_t *>(&t);
  vec.push_back(p[0]);
  vec.push_back(p[1]);
  vec.push_back(p[2]);
  vec.push_back(p[3]);
  return vec;
}

template <>
inline int cevy::deserialize<int>(std::vector<uint8_t> &vec) {
  int t;
  auto p = reinterpret_cast<uint8_t *>(&t);
  for (size_t i = serialized_size<int>::value; i--;) {
    p[i] = vec.back();
    vec.pop_back();
  }
  return t;
}

template <>
struct cevy::serialized_size<float> : public std::integral_constant<size_t, sizeof(float)> {};

template <>
inline std::vector<uint8_t> &cevy::serialize<float>(std::vector<uint8_t> &vec, const float &t) {
  auto p = reinterpret_cast<const uint8_t *>(&t);
  vec.push_back(p[0]);
  vec.push_back(p[1]);
  vec.push_back(p[2]);
  vec.push_back(p[3]);
  return vec;
}

template <>
inline float cevy::deserialize<float>(std::vector<uint8_t> &vec) {
  float t;
  auto p = reinterpret_cast<uint8_t *>(&t);
  for (size_t i = serialized_size<float>::value; i--;) {
    p[i] = vec.back();
    vec.pop_back();
  }
  return t;
}
