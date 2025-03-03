/*
** Agartha-Software, 2023
** C++evy
** File description:
** Asset Manager
*/

#pragma once

#include "App.hpp"
#include "Model.hpp"
#include "PbrMaterial.hpp"
#include "Plugin.hpp"
#include "ecs.hpp"

#include <optional>
#include <vector>

void init_asset_manager(cevy::ecs::World &w);

namespace cevy::engine {
class AssetManager {
  public:
  template <typename Type>
  std::optional<Handle<Type>> get(const std::string name = "") {
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
    auto anys_found = this->anys.find(std::type_index(typeid(Type)));
    if (anys_found == this->anys.end()) {
      anys_found->second = cevy::make_any<std::vector<Handle<Type>>>();
    }

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
  std::unordered_map<std::type_index, std::unordered_map<std::string, size_t>> any_keys;
  std::unordered_map<std::type_index, cevy::any> anys; // any = std::vector<Asset>

  std::unordered_map<std::string, size_t> mesh_keys;
  std::vector<Handle<cevy::engine::Model>> meshes;
  std::unordered_map<std::string, size_t> texture_keys;
  std::vector<Handle<cevy::engine::Texture>> textures;
  std::unordered_map<std::string, size_t> material_keys;
  std::vector<Handle<cevy::engine::PbrMaterial>> materials;

  // std::vector<ShaderProgram> _shaders;
};

class AssetManagerPlugin : public ecs::Plugin {
  public:
  void build(ecs::App &app);
};
} // namespace cevy::engine
