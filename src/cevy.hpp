/*
** Agartha-Software, 2023
** C++evy
** File description:
** cevy
*/

#pragma once

#include "any_nc.hpp"
#include <functional>
#include <optional>

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

template <template <typename > typename M, typename T, typename R, typename F>
class Map {
  static inline constexpr M<R> map(M<T>&& mappable, F &&func);
};

template <typename T, typename R, typename F>
class Map<std::optional, T, R, F> {
public:
  static inline constexpr std::optional<R> map(std::optional<T>&& opt, F &&func) {
    if (opt) {
      R ret = func(std::forward<T>(opt.value()));
      return std::make_optional<R>(std::forward<R>(ret));;
    } else {
      return std::nullopt;
    }
  }
};

template <template <typename > typename M, typename T, typename F, typename R = typename std::invoke_result<F,T&&>::type>
inline constexpr M<R> map(M<T> &&mappable, F &&func) {
  return Map<M, T, R, F>::map(std::forward<M<T>>(mappable), std::forward<F>(func));
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

// note: here name-spaces are being forward declared;
// they will be expanded by their relevant files;
