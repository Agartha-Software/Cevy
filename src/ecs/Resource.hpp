/*
** Agartha-Software, 2023
** C++evy
** File description:
** Queries
*/

#pragma once

#include "cevy.hpp"
#include "ecs.hpp"
#include <optional>
#include <typeindex>
#include <unordered_map>

namespace cevy::ecs {
class ResourceManager;
}

template <typename Content>
class cevy::ecs::Resource {
  private:
  Content &_content;
  friend class cevy::ecs::ResourceManager;
  Resource(Content &content) : _content(content) {};

  public:
  using value = Content;
  operator Content() const { return _content; };
  const Content *operator->() const { return &_content; };
  Content *operator->() { return &_content; };
  operator Content &() { return _content; };
  Content &get() { return _content; };
};

template <class T>
struct is_resource : public std::false_type {};

template <typename T>
struct is_resource<cevy::ecs::Resource<T>> : public std::true_type {};

template <typename T>
struct is_resource<std::optional<cevy::ecs::Resource<T>>> : public std::true_type {};

namespace cevy::ecs {
class ResourceManager {
  private:
  using any = std::any_nc;
  template <typename T, typename... Args>
  static inline auto
  make_any(Args &&...args) -> decltype(std::make_any_nc<T>(std::forward<Args>(args)...)) {
    return std::make_any_nc<T>(std::forward<Args>(args)...);
  }
  using resource_type = any;

  std::unordered_map<std::type_index, resource_type> _resources_map;

  public:
  void clear_resources() { _resources_map.clear(); }

  template <typename Content>
  void insert_resource(const Content &value) {
    _resources_map.insert_or_assign(std::type_index(typeid(Content)), make_any<Content>(value));
  }

  template <typename R, typename... Params>
  void emplace_resource(Params &&...params) {
    _resources_map.insert_or_assign(std::type_index(typeid(R)),
                                    make_any<R>(std::forward<Params &&>(params)...));
  }

  template <typename Content>
  std::optional<Content> remove_resource() {
    auto found = _resources_map.find(std::type_index(typeid(Content)));

    if (found != _resources_map.end()) {
      auto extracted = _resources_map.extract(found);
      Content val = std::any_cast<Content &&>(extracted.mapped());
      return std::optional<Content>(val);
    }
    return std::nullopt;
  }

  template <typename Content>
  Content &resource() {
    return std::any_cast<Content &>(_resources_map.at(std::type_index(typeid(Content))));
  }

  template <typename Content>
  const Content &resource() const {
    return std::any_cast<const Content &>(_resources_map.at(std::type_index(typeid(Content))));
  }

  template <typename Content>
  std::optional<cevy::ecs::Resource<Content>> get_resource() const {
    auto found = _resources_map.find(std::type_index(typeid(Content)));

    if (found != _resources_map.end())
      return cevy::ecs::Resource(std::any_cast<Content &>(found->second));
    return std::nullopt;
  }

  template <typename Content>
  std::optional<cevy::ecs::Resource<Content>> get_resource() {
    auto found = _resources_map.find(std::type_index(typeid(Content)));

    if (found != _resources_map.end())
      return cevy::ecs::Resource(std::any_cast<Content &>(found->second));
    return std::nullopt;
  }

  template <typename Content>
  cevy::ecs::Resource<Content> get() {
    auto &r = std::any_cast<Content &>(_resources_map.at(std::type_index(typeid(Content))));
    return cevy::ecs::Resource(r);
  }

  template <typename Content>
  bool contains_resource() const {
    return _resources_map.find(std::type_index(typeid(Content))) != _resources_map.end();
  }
};
} // namespace cevy::ecs
