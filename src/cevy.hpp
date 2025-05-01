/*
** Agartha-Software, 2023
** C++evy
** File description:
** cevy
*/

#pragma once

#include "any_nc.hpp"
#include <cstdint>
#include <cstring>
#include <functional>
#include <optional>
#include <sstream>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <utility>

#if defined(__clang__) || defined(__GNUC__)
#include <cxxabi.h>
#endif

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

template <typename T, typename Find, typename Replace>
using replace = std::conditional<std::is_same_v<T, Find>, Replace, T>;

template <typename R, typename... Args>
constexpr std::function<R(Args...)> make_function(R (&&func)(Args...)) {
  return std::function<R(Args...)>(func);
};


namespace std {
namespace detail {
template<typename ...Ts>
struct hash_pack_impl;


template<typename T, typename ...Ts>
struct hash_pack_impl<T, Ts...> {
  #if defined(__clang__) || defined(__GNU__)
  __attribute__((flatten))
  #endif
  inline size_t operator()(const T &t, const Ts&... ts) const {
    size_t h = hash_pack_impl<Ts...>()(ts...);
    return 0x9e3779b9 + ::std::hash<T>()(t) + ((h << 5) + (h >> 3));
  }
};


template<typename T>
struct hash_pack_impl<T> {
  size_t operator()(const T &t) const {
    return hash<T>()(t);
  }
};
}

template<typename ...Ts>
struct hash_pack {
  size_t operator()(const Ts &...ts) const {
    return detail::hash_pack_impl<Ts...>()(ts...);
  }
};


template<typename ...Ts>
struct hash<tuple<Ts...>> {
    size_t operator()(const tuple<Ts...> &ts) const {
    return apply(hash_pack<Ts...>(), ts);
  }
};
}


/// @brief contains all of the engine bits
namespace cevy {

template <typename M_t, typename T, typename F, typename M_r>
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

template <template <typename...> typename M, typename... M_as,
          typename T = typename M<M_as...>::value_type, typename F,
          typename R = typename std::invoke_result<F, T &&>::type,
          typename M_r = /* M<replace<M_as, T, R>...>> */ M<R>>
inline constexpr M<R> map(M<M_as...> &&mappable, F &&func) {
  return Map<M<M_as...>, T, F, M_r>::map(std::forward<M<T>>(mappable), std::forward<F>(func));
}

template <template <typename...> typename M, typename... M_as,
          typename T = typename M<M_as...>::value_type, typename F,
          typename R = typename std::invoke_result<F, T &&>::type,
          typename M_r = /* M<replace<M_as, T, R>...>> */ M<R>>
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

namespace detail {
template <bool StripScope = true>
inline std::string demangle(const char *cstr) {
#if defined(__clang__) || defined(__GNUC__) && !defined(NO_DEMANGLE)
  char *demangled = abi::__cxa_demangle(cstr, 0, 0, NULL);
  char *stripped = nullptr;
  if constexpr (StripScope) {
    stripped = strrchr(demangled, ':');
    stripped += (stripped != nullptr);
  }
  std::string demangled_clean = std::string(stripped ? stripped : demangled);
  free(demangled);

  return demangled_clean;
#else
  const char *stripped = nullptr;
  if constexpr (StripScope) {
    stripped = strrchr(cstr, ':');
    stripped += (stripped != nullptr);
  } else {
    stripped = strrchr(cstr, ' ');
    stripped += (stripped != nullptr);
  }
  return std::string(stripped ? stripped : cstr);
#endif
}
} // namespace detail

inline std::string pretty_type_name(const std::type_index& type) {
  return detail::demangle(type.name());
}

inline std::string pretty_type_name(const std::type_info& type) {
  return detail::demangle(type.name());
}

template<typename T>
std::string reflect() {
  return pretty_type_name(typeid(T));
}

template<typename T>
std::string reflect(const T&) {
  return reflect<T>() + "::<content not implemented>";
}

template<>
inline std::string reflect<uint64_t>(const uint64_t &i) {
  return (std::stringstream() << i << "ul").str();
}

template<>
inline std::string reflect<int64_t>(const int64_t &i) {
  return (std::stringstream() << i << "il").str();
}

template<>
inline std::string reflect<uint32_t>(const uint32_t &i) {
  return (std::stringstream() << i << "u").str();
}

template<>
inline std::string reflect<int32_t>(const int32_t &i) {
  return (std::stringstream() << i << "i").str();
}

template<>
inline std::string reflect<uint16_t>(const uint16_t &i) {
  return (std::stringstream() << i << "us").str();
}

template<>
inline std::string reflect<int16_t>(const int16_t &i) {
  return (std::stringstream() << i << "is").str();
}

template<>
inline std::string reflect<uint8_t>(const uint8_t &i) {
  return (std::stringstream() << i << "uc").str();
}

template<>
inline std::string reflect<int8_t>(const int8_t &i) {
  return (std::stringstream() << i << "ic").str();
}

template<>
inline std::string reflect<double>(const double &i) {
  return (std::stringstream() << i << "d").str();
}

template<>
inline std::string reflect<float>(const float &i) {
  return (std::stringstream() << i << "f").str();
}


} // namespace cevy

// note: here name-spaces are being forward declared;
// they will be expanded by their relevant files;
