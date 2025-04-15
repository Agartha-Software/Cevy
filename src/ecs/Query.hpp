/*
** Agartha-Software, 2023
** C++evy
** File description:
** Queries
*/

#pragma once

#include <algorithm>
#include <array>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <type_traits>
#include <utility>

#include "Entity.hpp"
#include "SparseVector.hpp"
#include "cevy.hpp"
#include "ecs.hpp"

namespace cevy::ecs {

template <class T>
struct is_query : public std::false_type {};

template <typename... T>
struct is_query<cevy::ecs::Query<T...>> : public std::true_type {};

template <typename Type>
struct is_optional : std::false_type {};

template <typename Type>
struct is_optional<std::optional<Type>> : std::true_type {};

template <class X>
using inner_optional = typename X::value_type;

template <typename Type>
using remove_optional = eval_cond_t<is_optional<Type>::value, inner_optional, Type, Type>;

template <typename T, typename V>
struct same_const : std::conditional<std::is_const_v<T>, const V, V> {};

template <typename T, typename V>
using same_const_t = typename same_const<T, V>::type;

template <class... T>
class iterator {
  static_assert(std::conjunction_v<std::negation<std::is_same<T, Entity>>...>,
                "Entity must only be first in a request");

  friend class Entity;
  template <class...>
  friend class Query;

  public:
  using value_type = std::tuple<T &...>;
  using reference = value_type;
  using pointer = void;
  using difference_type = size_t;
  using iterator_category = std::forward_iterator_tag;
  template <typename _T>
  using iterator_for_t = std::conditional_t<
      std::is_const_v<_T>,
      typename SparseVector<std::remove_cv_t<remove_optional<_T>>>::const_iterator,
      typename SparseVector<std::remove_cv_t<remove_optional<_T>>>::iterator>;
  using iterator_tuple = std::tuple<iterator_for_t<T>...>;

  iterator(iterator_tuple const &it_tuple, size_t max, size_t idx = 0)
      : current(it_tuple), _max(max), _idx(idx), _entity(_idx) {
    sync();
  };

  public:
  static iterator begin(World &w, size_t size);
  static iterator end(World &w, size_t size);

  template <typename Current>
  static void resize_optional(SparseVector<std::remove_cv_t<remove_optional<Current>>> &c,
                              size_t n) {
    if constexpr (is_optional<Current>::value) {
      c.resize(std::max(c.size(), n));
    }
  }

  template <typename Current>
  static size_t
  _compute_a_size(const SparseVector<std::remove_cv_t<remove_optional<Current>>> &container) {
    if (is_optional<Current>::value) {
      return SIZE_MAX;
    } else {
      return container.size();
    }
  }

  static size_t _compute_size(World &w, size_t nb_e);

  iterator(iterator const &z) : current(z.current), _max(z._max), _idx(z._idx), _entity(_idx) {};

  iterator operator++() {
    incr_all();
    return *this;
  };
  iterator operator++(int) {
    auto old = *this;
    incr_all();
    return old;
  };

  iterator &operator+=(size_t n) {
    incr_all(n);
    return *this;
  };

  iterator operator+(size_t n) {
    auto it = *this;
    return it += n;
  };

  value_type operator*() { return to_value(); };
  value_type operator->() { return to_value(); };

  friend bool operator==(iterator const &lhs, iterator const &rhs) { return lhs._idx == rhs._idx; };
  friend bool operator!=(iterator const &lhs, iterator const &rhs) { return lhs._idx != rhs._idx; };

  protected:
  void incr_all(size_t n = 1) {
    if (_idx >= _max)
      return;
    _idx += n;
    ((std::get<iterator_for_t<T>>(current) += n), ...);
    sync();
  }

  void incr_exact(size_t n = 1) {
    if (_idx >= _max)
      return;
    _idx += n;
    ((std::get<iterator_for_t<T>>(current) += n), ...);
  }

  void sync() {
    if (_idx >= _max)
      return;
    while (_idx < _max && !all_set()) { // NOTE - check to choose <= or <
      _idx += 1;
      ((std::get<iterator_for_t<T>>(current) += 1), ...);
    }
  }

  template <typename Current>
  bool is_set() {
    if (_idx >= _max)
      return false;
    if constexpr (is_optional<Current>::value) {
      return true;
    } else {
      return std::get<iterator_for_t<Current>>(current)->has_value();
    }
  }

  bool all_set() { return (true && ... && is_set<T>()); }

  template <typename Current>
  Current &a_value() {
    if constexpr (std::is_same<Current, Entity>::value) {
      return Entity(_idx);
    } else if constexpr (is_optional<Current>::value) {
      return *std::get<iterator_for_t<Current>>(current);
    } else {
      return std::get<iterator_for_t<Current>>(current)->value();
    }
  }

  const value_type to_value() { return value_type {a_value<T>()...}; }

  protected:
  iterator_tuple current;
  size_t _max;
  size_t _idx;
  Entity _entity;
};

template <typename... T>
class iterator<Entity, T...> : public iterator<T...> {
  static_assert(std::conjunction_v<std::negation<std::is_same<T, Entity>>...>,
                "Entity must only be first in a request");

  public:
  using value_type = std::tuple<Entity, T &...>;
  template <typename _T>
  using iterator_for_t = std::conditional_t<
      std::is_const_v<_T>,
      typename SparseVector<std::remove_cv_t<remove_optional<_T>>>::const_iterator,
      typename SparseVector<std::remove_cv_t<remove_optional<_T>>>::iterator>;
  using iterator_tuple = std::tuple<iterator_for_t<T>...>;

  iterator(iterator_tuple const &it_tuple, size_t max, size_t idx = 0)
      : iterator<T...>(it_tuple, max, idx) {};

  const value_type to_value() {
    return value_type {Entity(iterator<T...>::_idx), iterator::template a_value<T>()...};
  }

  static iterator begin(World &w, size_t size);
  static iterator end(World &w, size_t size);

  iterator operator+(size_t n) {
    auto it = *this;
    return it += n;
  };

  iterator &operator+=(size_t n) {
    iterator<T...>::incr_all(n);
    return *this;
  };

  value_type operator*() { return to_value(); };
  value_type operator->() { return to_value(); };
};

template <class... T>
class Query {
  using Containers = std::tuple<SparseVector<std::remove_cv_t<remove_optional<T>>>...>;

  public:
  using iterator_t = iterator<T...>;
  using iterator_tuple = typename iterator_t::iterator_tuple;

  static Query<T...> query(World &w) { return Query<T...>(w); }

  Query(World &w);

  iterator_t begin() { return _begin; };
  iterator_t end() { return _end; };

  const iterator_t begin() const { return _begin; };
  const iterator_t end() const { return _end; };

  private:
  public:
  size_t size() { return _size; }

  typename iterator_t::value_type single() {
    auto it = begin();
    it.sync();
    return *it;
  }

  std::optional<typename iterator_t::value_type> get_single() {
    auto it = begin();
    it.sync();
    if (it.all_set()) {
      return *it;
    } else {
      return std::nullopt;
    }
  }

  std::optional<typename iterator_t::value_type> get(const Entity &id) {
    auto at = begin();
    while (at._idx != id && at != _end) {
      at += 1;
    }
    if (at.all_set()) {
      return std::make_optional(at.to_value());
    } else {
      return std::nullopt;
    }
  }

  private:
  template <size_t N>
  typename iterator_t::value_type progress_it(iterator_t &it) {
    auto last = it;
    it++;
    return *last;
  }

  template <size_t N, size_t... I>
  std::array<typename iterator_t::value_type, N> multiple_impl(std::index_sequence<I...>) {
    auto it = begin();
    return {progress_it<I>(it)...};
  }

  public:
  template <size_t N, typename Indicies = std::make_index_sequence<N>>
  std::array<typename iterator_t::value_type, N> multiple() {
    return multiple_impl<N>(Indicies {});
  }

  template <size_t N>
  std::optional<std::array<typename iterator_t::value_type, N>> get_multiple() {
    if (_size < N) {
      return std::nullopt;
    }
    return multiple<N>();
  }

  private:
  size_t _size;
  iterator_t _begin;
  iterator_t _end;
};

} // namespace cevy::ecs
