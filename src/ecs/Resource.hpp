/*
** Agartha-Software, 2023
** C++evy
** File description:
** Queries
*/

#pragma once

#include "ecs.hpp"
#include <any>
#include <optional>
#include <typeindex>
#include <unordered_map>

namespace cevy::ecs {
class ResourceManager;
}

template <typename content>
class cevy::ecs::Resource {
  private:
  content &_content;
  friend class cevy::ecs::ResourceManager;
  Resource(content &content) : _content(content){};

  public:
  using value = content;
  operator content() const { return _content; };
  operator content &() { return _content; };
  content &get() { return _content; };
};

template <class T>
struct is_resource : public std::false_type {};

template <typename T>
struct is_resource<cevy::ecs::Resource<T>> : public std::true_type {};

namespace cevy::ecs {
class ResourceManager {
  private:
  using resource_type = std::any;

  std::unordered_map<std::type_index, resource_type> _resources_map;

  public:
  void clear_resources() { _resources_map.clear(); }

  template <typename content>
  void insert_resource(const content &value) {
    auto a = std::make_any<content>(value);

    _resources_map[std::type_index(typeid(content))] = a;
  }

  template <typename content>
  std::optional<content> remove_resource() {
    auto it = _resources_map.find(std::type_index(typeid(content)));

    if (it != _resources_map.end()) {
      content val = std::any_cast<content>(_resources_map[std::type_index(typeid(content))]);

      _resources_map.erase(it);
      return std::optional<content>(val);
    }
    return std::nullopt;
  }

  template <typename content>
  content &resource() {
    return std::any_cast<content &>(_resources_map[std::type_index(typeid(content))]);
  }

  template <typename content>
  const content &resource() const {
    return std::any_cast<const content &>(_resources_map.at(std::type_index(typeid(content))));
  }

  template <typename content>
  std::optional<std::reference_wrapper<content>> get_resource() {
    auto it = _resources_map.find(std::type_index(typeid(content)));

    if (it != _resources_map.end())
      return std::any_cast<content &>(_resources_map[std::type_index(typeid(content))]);
    return std::nullopt;
  }

  template <typename content>
  cevy::ecs::Resource<content> get() {
    return cevy::ecs::Resource(
        std::any_cast<content &>(_resources_map[std::type_index(typeid(content))]));
  }

  template <typename content>
  bool contains_resource() {
    return _resources_map.find(std::type_index(typeid(content))) != _resources_map.end();
  }
};
} // namespace cevy::ecs
