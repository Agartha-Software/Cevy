/*
** Agartha-Software, 2023
** C++evy
** File description:
** ecs event
*/

#pragma once

#include <iostream>
#include <iterator>
#include <tuple>
#include <vector>

namespace cevy::ecs {
class World;

template <typename T>
class EventWriter;

template <typename T>
class Event {
  public:
  using EventWriterId = size_t;
  using data_iterator_type = typename std::vector<std::tuple<T, EventWriterId>>::iterator;

  std::vector<std::tuple<T, EventWriterId>> event_queue{};
};

template <typename T>
class EventWriter {
  private:
  friend class World;
  Event<T> &_event_access;
  size_t _idx;

  EventWriter(Event<T> &event_access, size_t idx) : _event_access(event_access), _idx(idx) {}

  public:
  using value_type = T;

  void clear() {
    for (int i = _event_access.event_queue.size() - 1; i >= 0; i--)
      if (std::get<1>(_event_access.event_queue.at(i)) == _idx)
        _event_access.event_queue.erase(_event_access.event_queue.begin() + i);
  }

  void send(const T &event) { _event_access.event_queue.push_back(std::make_tuple(event, _idx)); }
};

template <typename T>
class EventReader {
  private:
  friend class World;

  EventReader(const Event<T> &event_access) : raw_data(event_access) {}

  public:
  using value_type = T;

  const Event<T> &raw_data; // TODO: replace by a query like iterator

  class iterator {
    public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T *;
    using reference = T &;

    using parent_iterator_type = typename std::vector<std::tuple<T, size_t>>::const_iterator;

    parent_iterator_type inner_iter;

    iterator(parent_iterator_type &&inner_iter) : inner_iter(inner_iter) {}

    iterator operator++() {
      inner_iter++;
      return *this;
    };

    iterator &operator++(int) {
      auto old = *this;
      inner_iter++;
      return old;
    };

    value_type operator*() { return std::get<0>(*inner_iter); };
    value_type operator->() { return std::get<0>(*inner_iter); };

    operator reference() const { return std::get<0>(*inner_iter); };

    friend bool operator==(iterator const &lhs, iterator const &rhs) {
      return lhs.inner_iter == rhs.inner_iter;
    };
    friend bool operator!=(iterator const &lhs, iterator const &rhs) {
      return lhs.inner_iter != rhs.inner_iter;
    };
  };

  public:
  const iterator cbegin() const { return iterator(raw_data.event_queue.cbegin()); };
  const iterator cend() const { return iterator(raw_data.event_queue.cend()); };
  const iterator begin() const { return iterator(raw_data.event_queue.cbegin()); };
  const iterator end() const { return iterator(raw_data.event_queue.cend()); };
  const iterator read() const { return iterator(raw_data.event_queue.begin()); };
};
} // namespace cevy::ecs

template <class T>
struct is_event_writer : public std::false_type {};

template <typename T>
struct is_event_writer<cevy::ecs::EventWriter<T>> : public std::true_type {};

template <class T>
struct is_event_reader : public std::false_type {};

template <typename T>
struct is_event_reader<cevy::ecs::EventReader<T>> : public std::true_type {};
