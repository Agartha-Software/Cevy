/*
** Agartha-Software, 2023
** C++evy
** File description:
** Asset Manager
*/

#pragma once

#include "App.hpp"
#include "Handle.hpp"
#include "Mesh.hpp"
#include "Plugin.hpp"
#include "cevy.hpp"
#include "ecs.hpp"

#include <functional>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

#include "App.hpp"
#include "Mesh.hpp"
#include "Plugin.hpp"
#include "ecs.hpp"

void init_asset_manager(cevy::ecs::World &w);

namespace cevy::engine {
class AssetManager {
  public:
  template <typename Type>
  std::optional<Handle<Type>> get(const std::string name = "") {
    auto in = this->lookup<Type>(name);
    if (in)
      return in;
    return this->factory<Type>(name);
  }

  template <typename Type>
  std::optional<Handle<Type>> lookup(const std::string name = "") {
    auto anys_found = this->anys.find(std::type_index(typeid(Type)));
    if (anys_found == this->anys.end()) {
      return std::nullopt;
    }

    std::vector<Handle<Type>> &handles =
        std::any_cast<std::vector<Handle<Type>> &>(anys_found->second);
    auto &keys = this->any_keys.at(std::type_index(typeid(Type)));

    auto found = keys.find(name);
    if (found != keys.end()) {
      return handles.at(found->second);
    } else {
      return std::nullopt;
    }
  }

  template <typename Type>
  Handle<Type> load(Type &&asset, const std::string name = "") {
    auto [anys_found, is_new] =
        this->anys.try_emplace(std::type_index(typeid(Type)), std::vector<Handle<Type>>());

    std::vector<Handle<Type>> &handles =
        std::any_cast<std::vector<Handle<Type>> &>(anys_found->second);
    size_t idx = handles.size();
    if (name != "") {
      auto &keys = this->any_keys[std::type_index(typeid(Type))];
      auto found = keys.find(name);
      if (found != keys.end()) {
        return handles.at(found->second) = std::move(Handle<Type>(std::forward<Type>(asset)));
      } else {
        keys[name] = idx;
      }
    }

    return handles.emplace_back(
        std::forward<Handle<Type>>(Handle<Type>(std::forward<Type>(asset))));
  }

  protected:
  template <template <typename T> typename Windower, typename Renderer>
  friend class Engine;

  template <typename T>
  void add_factory(const std::string &key, std::function<T()> &&func) {
    using Func = std::function<T()>;
    this->factories.emplace(
        std::make_pair(key, std::move(cevy::make_any<Func>(std::forward<Func>(func)))));
  }

  template <typename T>
  std::optional<Handle<T>> factory(const std::string &key) {
    using Func = std::function<T()>;
    auto found = this->factories.find(key);
    {
      if (found != this->factories.end()) {
        auto &func = std::any_cast<Func &>(found->second);
        return this->load(std::forward<T>(func()), key);
      }
    }
    return std::nullopt;
  }

  std::unordered_map<std::type_index, std::unordered_map<std::string, size_t>> any_keys;
  std::unordered_map<std::type_index, cevy::any> anys; // any = std::vector<Asset>

  std::unordered_map<std::string, size_t> mesh_keys;
  std::vector<Handle<cevy::engine::Mesh>> meshes;

  std::unordered_map<std::string, cevy::any> factories;
  // std::vector<ShaderProgram> _shaders;
};

class AssetManagerPlugin : public ecs::Plugin {
  public:
  void build(ecs::App &app);
};
} // namespace cevy::engine

template <>
cevy::engine::Handle<cevy::engine::Mesh>
cevy::engine::AssetManager::load(cevy::engine::Mesh &&model, std::string name);

template <>
std::optional<cevy::engine::Handle<cevy::engine::Mesh>>
cevy::engine::AssetManager::get<cevy::engine::Mesh>(std::string name);
