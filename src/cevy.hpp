/*
** Agartha-Software, 2023
** C++evy
** File description:
** cevy
*/

#pragma once

#include "any_nc.hpp"
#include <cstdint>
#include <functional>
#include <glm/detail/qualifier.hpp>
#include <optional>
#include <utility>

template <typename T>
using ref = std::reference_wrapper<T>;

template <class T>
using option = std::optional<T>;

template <bool, template <class...> class, class, class Else>
struct eval_cond {
  using type = Else;
};

/// @brief Template structure to allow conditional template without interpretation of the wrong case
template <template <class...> class Z, class X, class Else>
struct eval_cond<true, Z, X, Else> {
  using type = Z<X>;
};

template <bool test, template <class...> class Z, class X, class Else>
using eval_cond_t = typename eval_cond<test, Z, X, Else>::type;

template<typename T, typename Find, typename Replace>
using replace = std::conditional<std::is_same_v<T, Find>, Replace, T>;

/// @brief True if all parameter pack is true
template <typename... Args>
constexpr bool all(Args... args) {
  return (... && args);
}

/// @brief True if any element in the parameter pack is true
template <typename... Args>
constexpr bool any() {
  return (... || Args::value);
};

template <typename... Args>
constexpr size_t sum(Args... args) {
  return (0 + ... + args);
};

template <typename R, typename... Args>
constexpr std::function<R(Args...)> make_function(R (&&func)(Args...)) {
  return std::function<R(Args...)>(func);
};

/// @brief contains all of the engine bits
namespace cevy {

template <typename M_t, typename T, typename F,
          typename M_r>
class Map {
  public:

  static inline constexpr M_r map(M_t &&mappable, F &&func) {
    M_r ret;
    auto inserter = std::back_inserter(ret);
    for (auto x : mappable) {
      inserter = func(std::forward<T>(x));
    }
    return std::forward<M_r>(ret);
  }

  static inline constexpr M_r map(const M_t &mappable, F &&func) {
    M_r ret;
    auto inserter = std::back_inserter(ret);
    for (auto x : mappable) {
      inserter = func(std::forward<T>(x));
    }
    return std::forward<M_r>(ret);
  }
};

template <typename F, typename T, typename R>
class Map<std::optional<T>, T, F, std::optional<R>> {
  public:
  static inline constexpr std::optional<R> map(std::optional<T> &&opt, F &&func) {
    if (opt) {
      R ret = func(std::forward<T>(opt.value()));
      return std::make_optional<R>(std::forward<R>(ret));
      ;
    } else {
      return std::nullopt;
    }
  }
};

template <template <typename...> typename M, typename ...M_as, typename T = typename M<M_as...>::value_type, typename F,
          typename R = typename std::invoke_result<F, T &&>::type, typename M_r = /* M<replace<M_as, T, R>...>> */ M<R>>
inline constexpr M<R> map(M<M_as...> &&mappable, F &&func) {
  return Map<M<M_as...>, T, F, M_r>::map(std::forward<M<T>>(mappable), std::forward<F>(func));
}

template <template <typename...> typename M, typename ...M_as, typename T = typename M<M_as...>::value_type, typename F,
          typename R = typename std::invoke_result<F, T &&>::type, typename M_r = /* M<replace<M_as, T, R>...>> */ M<R>>
inline constexpr M<R> map(const M<M_as...> &mappable, F &&func) {
  return Map<M<M_as...>, T, F, M_r>::map(mappable, std::forward<F>(func));
}

/// @brief holds the entity components system
namespace ecs {};

/// @brief hold engine mechanics, depends on ecs
namespace engine {};

/// @brief hold physics mechanism, depends on ecs
namespace physics {};

using any = std::any_nc;
template <typename T, typename... Args>
auto make_any(Args &&...args) -> decltype(std::make_any_nc<T>(std::forward<Args>(args)...)) {
  return std::make_any_nc<T>(std::forward<Args>(args)...);
}

} // namespace cevy

namespace glm {
using vec4u8 = vec<4, uint8_t>;
}

// note: here name-spaces are being forward declared;
// they will be expanded by their relevant files;
