/*
** Agartha-Software, 2023
** C++evy
** File description:
** Asset Manager
*/

#include "AssetManager.hpp"
#include "Asset.hpp"
#include "PbrMaterial.hpp"
#include "engine.hpp"
#include <optional>

using cevy::engine::Asset;
using cevy::engine::AssetManager;
using cevy::engine::AssetManagerPlugin;
using cevy::engine::Handle;
using cevy::engine::Mesh;
using cevy::engine::PbrMaterial;

void init_asset_manager(cevy::ecs::World &w) {
  auto asset_manager = w.get_resource<AssetManager>();
  if (asset_manager) {
    w.insert_resource(Asset<Mesh>(asset_manager->get()));
    w.insert_resource(Asset<PbrMaterial>(asset_manager->get()));
  }
}

void AssetManagerPlugin::build(ecs::App &app) {
  app.init_resource(AssetManager());
  app.add_systems<PostStartupRenderStage>(init_asset_manager);
  app.init_component<Handle<Mesh>>();
  app.init_component<Handle<PbrMaterial>>();
}

template <>
Handle<Mesh> AssetManager::load(Mesh &&model, std::string name) {
  size_t idx = this->meshes.size();
  if (name != "") {
    auto found = this->mesh_keys.find(name);
    if (found != this->mesh_keys.end()) {
      return this->meshes.at(found->second) = Handle<Mesh>(std::forward<Mesh>(model));
    } else {
      this->mesh_keys[name] = idx;
    }
  }

  return this->meshes.emplace_back(Handle<Mesh>(std::forward<Mesh>(model)));
}

template <>
std::optional<Handle<Mesh>> AssetManager::lookup<Mesh>(std::string name) {
  auto found = this->mesh_keys.find(name);
  if (found != this->mesh_keys.end()) {
    return this->meshes.at(found->second);
  } else {
    return std::nullopt;
  }
}
