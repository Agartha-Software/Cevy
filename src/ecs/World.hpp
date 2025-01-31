/*
** Agartha-Software, 2023
** C++evy
** File description:
** World
*/

#pragma once

/**
 * @file
 * @author several
 *
 * @section DESCRIPTION
 * World. Holds the actual components and entities
 */

#include "Entity.hpp"
#include "Event.hpp"
#include "Resource.hpp"
#include "SparseVector.hpp"
#include "cevy.hpp"

#include <any>
#include <cstddef>
#include <functional>
#include <optional>
#include <queue>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

#include "Query.hpp"
#include "ecs.hpp"

template <class T>
struct is_world : public std::false_type {};

template <>
struct is_world<cevy::ecs::World &> : public std::true_type {};

template <>
struct is_world<const cevy::ecs::World &> : public std::true_type {};

template <typename... T>
struct Or : std::integral_constant<bool, any<T...>()> {};

namespace cevy::ecs {
class Commands;
} // namespace cevy::ecs

template <class T>
struct is_commands : public std::false_type {};

template <>
struct is_commands<cevy::ecs::Commands> : public std::true_type {};

/**
 * Stores Entities, Components (and resources), and exposes operations
 *
 * Each entity has Components, which must first each be registered to the World
 * An Entity can only have one instance of a Component
 */
class cevy::ecs::World {
  using value_type = World;

  public:
  struct EntityWorldRef {
    World &world;
    Entity entity;

    // EntityWorldRef insert(Bundle& b);

    template <typename... Ts>
    EntityWorldRef insert(Ts... args);

    template <typename... Ts>
    EntityWorldRef remove();

    template <typename T>
    T &get();

    template <typename T>
    bool contains();

    Entity id();

    operator Entity &();
  };

  using erase_access = std::function<void(World &, Entity const &)>;
  using command = std::function<void(World &)>;
  using component_data = std::tuple<std::any, erase_access>;

  friend class cevy::ecs::Scheduler;
  friend class cevy::ecs::Commands;
  friend class EntityWorldRef;
  friend class cevy::ecs::EntityCommands;

  private:
  std::queue<command> _command_queue;

  /* Bevy-compliant */
  public:
  /// @brief Id refering to a specific component
  using ComponentId = std::type_index;
  using Id = std::type_index;

  private:
  std::unordered_map<std::type_index, component_data> _components_arrays;
  SparseVector<Entity> _entities;
  cevy::ecs::ResourceManager _resource_manager;

  /* Bevy-compliant */
  public:
  /// get all entities
  SparseVector<Entity> &entities();

  /// get all entities
  const SparseVector<Entity> &entities() const;

  /// create a new empty entity
  EntityWorldRef spawn_empty();

  /// delete an entity and all its components
  bool despawn(const Entity &entity);

  /// clears all resources and entities
  void clear_all();

  /// clears all entities and their components
  void clear_entities();

  /// clears all resources registered to the world
  void clear_resources();

  /// spawn an entity with defined components
  template <typename... Components>
  EntityWorldRef spawn(Components... c) {
    return spawn_empty().insert(c...);
  }

  /*
  **
  ** RESOURCES
  **
  */

  // emplace a resource to the world by calling the contructor
  template <typename R, typename... Params>
  void init_resource(Params &&...p) {
    static_assert(std::is_constructible<R, Params&&...>::value,
                  "Resource must be constructible from Params");
    _resource_manager.emplace_resource<R>(std::forward<Params &&>(p)...);
  }

  // emplace a resource to the world by calling the move contructor
  template <typename R>
  void init_resource(R &&r) {
    static_assert(std::is_constructible<R, R>::value,
                  "Resource must be move or copy constructible");
    _resource_manager.emplace_resource<R>(std::forward<R>(r));
  }

  /// insert a resource to the world
  template <typename R>
  void insert_resource(const R &value) {
    _resource_manager.insert_resource(value);
  }

  /// remove a resource from this world
  template <typename R>
  std::optional<R> remove_resource() {
    return _resource_manager.remove_resource<R>();
  }

  /// true if the world holds this Resource
  template <typename R>
  bool contains_resource() const {
    return _resource_manager.contains_resource<R>();
  }

  /// access a given Resource
  template <typename R>
  R &resource() {
    return _resource_manager.resource<R>();
  }

  /// access a given Resource
  template <typename R>
  const R &resource() const {
    return _resource_manager.resource<R>();
  }

  /// access a given Resource, or None if it not in this world
  template <typename R>
  std::optional<Resource<R>> get_resource() const {
    return _resource_manager.get_resource<R>();
  }

  template <typename R>
  std::optional<Resource<R>> get_resource() {
    return _resource_manager.get_resource<R>();
  }

  /*
  **
  ** COMPONENTS
  **
  */

  /// register a component to the world
  template <typename T>
  ComponentId init_component() {
    erase_access f_e = [](World &reg, Entity const &Entity) {
      auto &cmpnts = reg.get_components<T>();
      if (Entity < cmpnts.size())
        cmpnts[Entity] = std::nullopt;
    };
    std::any a = std::make_any<SparseVector<T>>();

    _components_arrays.insert({std::type_index(typeid(T)), std::make_tuple(a, f_e)});

    return std::type_index(typeid(T));
  };

  template <typename Component>
  std::optional<Component> &add_component(Entity const &to, const Component &c) {
    auto &array = get_components<Component>();

    return array.insert_at(to, c);
  }

  template <typename Component, typename... Params>
  std::optional<Component> &emplace_component(Entity const &to, Params &&...p) {

    auto &array = get_components<Component>();
    return array.emplace_at(to, p...);
  }

  template <class Component>
  SparseVector<Component> &get_components() {
    auto id = std::type_index(typeid(Component));
    auto it = _components_arrays.find(id);

    if (it != _components_arrays.end()) {
      return std::any_cast<SparseVector<Component> &>(std::get<0>(_components_arrays[id]));
    }
    throw(std::runtime_error(
        std::string("Cevy/Ecs: Get unregisted component! ID: ").append(typeid(Component).name())));
  }

  template <class Component>
  SparseVector<Component> const &get_components() const {
    auto id = std::type_index(typeid(Component));
    auto it = _components_arrays.find(id);

    if (it != _components_arrays.end()) {
      return std::any_cast<SparseVector<Component> &>(std::get<0>(_components_arrays.at(id)));
    }
    throw(std::runtime_error(
        std::string("Cevy/Ecs: Get unregisted component! ID: ").append(typeid(Component).name())));
  }

  private:
  /// @deprecated No usage found
  template <typename Component>
  void remove_component(Entity const &from) {
    auto &array = get_components<Component>();
    if (from < array.size())
      array.erase(from);
  }

  /// get a Component T associated with a given Entity, or Nothing if no such
  /// Component
  template <typename T>
  std::optional<ref<T>> get_entity_component(Entity entity) {
    SparseVector<T> &v = get_components<T>();
    std::optional<T> optional = v[entity];

    if (optional)
      return std::optional<ref<T>>(std::ref(optional.value()));
    else
      return std::optional<ref<T>>(std::ref(std::nullopt));
  }

  /// get a Component T associated with a given Entity, or Nothing if no such
  /// Component
  /// @bug Const version don't work
  template <typename T>
  std::optional<ref<const T>> get_entity_component(Entity entity) const {
    SparseVector<T> const &v = get_components<T>();
    std::optional<const T> optional = v[entity];

    if (optional)
      return std::optional<ref<const T>>(std::ref(optional.value()));
    else
      return std::optional<ref<const T>>(std::ref(std::nullopt));
  }

  /// True if a Component T associated with a given Entity, or False if no such
  /// Component
  /// TODO: Add Consts
  template <typename T>
  bool entity_contains(Entity entity) {
    SparseVector<T> &v = get_components<T>();
    std::optional<T> optional = v[entity];

    return optional.has_value();
  }

  /*
  **
  ** QUERIES
  **
  */

  template <typename W, typename std::enable_if_t<is_world<W>::value, bool> = true>
  cevy::ecs::World &get_super(size_t) {
    return *this;
  }

  template <typename Q, typename std::enable_if_t<is_query<Q>::value, bool> = true>
  Q get_super(size_t) {
    return Q::query(*this);
  }

  template <typename R, typename std::enable_if_t<is_event_reader<R>::value, bool> = true>
  R get_super(size_t) {
    if (!contains_resource<Event<typename R::value_type>>())
      throw(std::runtime_error("Cevy/Ecs: Try to use EventReader on an unregisted event!"));

    return EventReader(resource<Event<typename R::value_type>>());
  }

  template <typename W, typename std::enable_if_t<is_event_writer<W>::value, bool> = true>
  W get_super(size_t system_id) {
    if (!contains_resource<Event<typename W::value_type>>())
      throw(std::runtime_error("Cevy/Ecs: Try to use EventWriter on an unregisted event!"));

    auto &res = resource<Event<typename W::value_type>>();

    if (res.event_queue.empty())
      return EventWriter(res, system_id);

    for (int i = res.event_queue.size() - 1; i >= 0; i--)
      if (std::get<1>(res.event_queue.at(i)) == system_id)
        res.event_queue.erase(res.event_queue.begin() + i);
    return EventWriter(res, system_id);
  }

  template <typename R, typename std::enable_if_t<is_resource<R>::value, bool> = true,
            typename std::enable_if_t<std::negation<is_optional<R>>::value, bool> = true>
  R get_super(size_t) {
    return _resource_manager.get<typename R::value>();
  }

  template <typename OR, typename R = typename OR::value_type,
            typename std::enable_if_t<is_resource<R>::value, bool> = true,
            typename std::enable_if_t<is_optional<OR>::value, bool> = true>
  OR get_super(size_t) {
    return _resource_manager.get_resource<R>();
  }

  template <typename C, typename std::enable_if_t<is_commands<C>::value, bool> = true>
  C get_super(size_t);

  template <class R, class... Args>
  R run_system(R (&&func)(Args...)) {
    static_assert(
        all(Or<is_query<Args>, is_world<Args>, is_resource<Args>, is_commands<Args>,
               is_event_reader<Args>, is_event_writer<Args>>()...),
        "type must be reference to query, world, commands, event reader, event writer or resource");
    auto sys = [&func, this]() mutable -> R { return func(get_super<Args>(0)...); };
    return sys();
  }

  // template <class... Args>
  // void run_system(void (&&func)(Args...)) {
  //   static_assert(
  //       all(Or<is_query<Args>, is_world<Args>, is_resource<Args>, is_commands<Args>,
  //              is_event_reader<Args>, is_event_writer<Args>>()...),
  //       "type must be reference to query, world, commands, event reader, event writer or
  //       resource");
  //   auto sys = [&func, this]() mutable { func(get_super<Args>(0)...); };
  //   sys();
  // }
public:
  template <class R, class... Args>
  R run_system(std::function<R(Args...)> func) {
    static_assert(
        all(Or<is_query<Args>, is_world<Args>, is_resource<Args>, is_commands<Args>,
               is_event_reader<Args>, is_event_writer<Args>>()...),
        "type must be reference to query, world, commands, event reader, event writer or resource");
    auto sys = [&func, this]() mutable -> R { return func(get_super<Args>(0)...); };
    return sys();
  }

  // template <class... Args>
  // void run_system(std::function<void (Args...)> func) {
  //   static_assert(
  //       all(Or<is_query<Args>, is_world<Args>, is_resource<Args>, is_commands<Args>,
  //              is_event_reader<Args>, is_event_writer<Args>>()...),
  //       "type must be reference to query, world, commands, event reader, event writer or
  //       resource");
  //   auto sys = [&func, this]() mutable { func(get_super<Args>(0)...); };
  //   sys();
  // }

  template <class GivenArgs, class R, class... Args>
  R run_system_with(R (&&func)(GivenArgs, Args...), GivenArgs&& given) {
    static_assert(
        all(Or<is_query<Args>, is_world<Args>, is_resource<Args>, is_commands<Args>,
               is_event_reader<Args>, is_event_writer<Args>>()...),
        "type must be reference to query, world, commands, event reader, event writer or resource");
    auto sys = [&func, this, &given]() mutable -> R { return func(std::forward<GivenArgs>(given), get_super<Args>(0)...); };
    return sys();
  }

  // template <class GivenArgs, class... Args>
  // void run_system_with(void (&&func)(GivenArgs, Args...), GivenArgs given) {
  //       static_assert(
  //       all(Or<is_query<Args>, is_world<Args>, is_resource<Args>, is_commands<Args>,
  //              is_event_reader<Args>, is_event_writer<Args>>()...),
  //       "type must be reference to query, world, commands, event reader, event writer or
  //       resource");
  //   auto sys = [&func, this, given]() mutable {
  //      func(given, get_super<Args>(0)...);
  //   };
  //   sys();
  // }

  template <class GivenArgs, class R, class... Args>
  R run_system_with(std::function<R(GivenArgs, Args...)> func, GivenArgs&& given) {
    static_assert(
        all(Or<is_query<Args>, is_world<Args>, is_resource<Args>, is_commands<Args>,
               is_event_reader<Args>, is_event_writer<Args>>()...),
        "type must be reference to query, world, commands, event reader, event writer or resource");
    auto sys = [&func, this, given]() mutable -> R { return func(std::forward<GivenArgs>(given), get_super<Args>(0)...); };
    return sys();
  }

  // template <class GivenArgs, class... Args>
  // void run_system_with(std::function<void(GivenArgs, Args...)> func, GivenArgs given) {
  //       static_assert(
  //       all(Or<is_query<Args>, is_world<Args>, is_resource<Args>, is_commands<Args>,
  //              is_event_reader<Args>, is_event_writer<Args>>()...),
  //       "type must be reference to query, world, commands, event reader, event writer or
  //       resource");
  //   auto sys = [&func, this, given]() mutable {
  //     func(given, get_super<Args>(0)...);
  //   };
  //   sys();
  // }
};

template <typename... Ts>
cevy::ecs::World::EntityWorldRef cevy::ecs::World::EntityWorldRef::insert(Ts... args) {
  (world.add_component(entity, std::forward<Ts>(args)), ...);
  return *this;
}

template <typename... Ts>
cevy::ecs::World::EntityWorldRef cevy::ecs::World::EntityWorldRef::remove() {
  (world.remove_component<Ts>(entity), ...);
  return *this;
}

template <typename T>
T &cevy::ecs::World::EntityWorldRef::get() {
  return world.get_entity_component<T>(entity);
}

template <typename T>
bool cevy::ecs::World::EntityWorldRef::contains() {
  return world.entity_contains<T>(entity);
}

template <typename... T>
cevy::ecs::iterator<T...> cevy::ecs::iterator<T...>::begin(World &w, size_t size) {
  return iterator<T...>(std::make_tuple(w.get_components<remove_optional<T>>().begin()...), size);
}

template <typename... T>
cevy::ecs::iterator<T...> cevy::ecs::iterator<T...>::end(World &w, size_t size) {
  return iterator<T...>(std::make_tuple(w.get_components<remove_optional<T>>().end()...), size,
                        size);
}

template <typename... T>
cevy::ecs::iterator<cevy::ecs::Entity, T...>
cevy::ecs::iterator<cevy::ecs::Entity, T...>::begin(World &w, size_t size) {
  return iterator<cevy::ecs::Entity, T...>(
      std::make_tuple(w.get_components<remove_optional<T>>().begin()...), size);
}

template <typename... T>
cevy::ecs::iterator<cevy::ecs::Entity, T...>
cevy::ecs::iterator<cevy::ecs::Entity, T...>::end(World &w, size_t size) {
  return iterator<cevy::ecs::Entity, T...>(
      std::make_tuple(w.get_components<remove_optional<T>>().end()...), size, size);
}

template <typename... T>
cevy::ecs::Query<T...>::Query(cevy::ecs::World &w)
    : _size(iterator_t::_compute_size(w, w.entities().size())), _begin(iterator_t::begin(w, _size)),
      _end(iterator_t::end(w, _size)){};

template <typename... T>
size_t cevy::ecs::iterator<T...>::_compute_size(World &w, size_t nb_e) {
  size_t current_size = 0;
  if ((... && is_optional<T>::value)) {
    current_size = nb_e;
  } else {
    std::bitset<sizeof...(T)> are_optional;
    size_t idx = 0;
    bool is_first = true;

    (are_optional.set(idx++, is_optional<T>::value), ...);
    idx = 0;
    (_compute_a_size(w.get_components<remove_optional<T>>(), current_size, is_first, idx,
                     are_optional),
     ...);
  }
  (resize_optional<T>(w.get_components<remove_optional<T>>(), current_size), ...);
  return current_size;
}
