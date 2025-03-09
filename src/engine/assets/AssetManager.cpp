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
using cevy::engine::Model;
using cevy::engine::PbrMaterial;
using cevy::engine::Texture;
using cevy::engine::TextureBuilder;

void init_asset_manager(cevy::ecs::World &w) {
  w.init_resource(AssetManager());
  auto asset_manager = w.get_resource<AssetManager>();
  if (asset_manager) {
    w.insert_resource(Asset<Model>(asset_manager->get()));
    w.insert_resource(Asset<PbrMaterial>(asset_manager->get()));
  }
}

void AssetManagerPlugin::build(ecs::App &app) {
  app.add_systems<PostStartupRenderStage>(init_asset_manager);
  app.init_component<Handle<Model>>();
  app.init_component<Handle<PbrMaterial>>();
}

template <>
Handle<PbrMaterial> AssetManager::load<PbrMaterial>(PbrMaterial &&material,
                                                    const std::string name) {
  size_t idx = this->materials.size();
  if (name != "") {
    auto found = this->material_keys.find(name);
    if (found != this->material_keys.end()) {
      return this->materials.at(found->second) =
                 Handle<PbrMaterial>(std::forward<PbrMaterial>(material));
    } else {
      this->material_keys[name] = idx;
    }
  }

  return this->materials.emplace_back(Handle<PbrMaterial>(std::forward<PbrMaterial>(material)));
}

template <>
Handle<Model> AssetManager::load(Model &&model, std::string name) {
  size_t idx = this->meshes.size();
  if (name != "") {
    auto found = this->mesh_keys.find(name);
    if (found != this->mesh_keys.end()) {
      return this->meshes.at(found->second) = Handle<Model>(std::forward<Model>(model));
    } else {
      this->mesh_keys[name] = idx;
    }
  }

  return this->meshes.emplace_back(Handle<Model>(std::forward<Model>(model)));
}

template <>
Handle<Texture> AssetManager::load(Texture &&texture, std::string name) {
  size_t idx = this->textures.size();
  if (name != "") {
    auto found = this->texture_keys.find(name);
    if (found != this->texture_keys.end()) {
      return this->textures.at(found->second) = Handle<Texture>(std::forward<Texture>(texture));
    } else {
      this->texture_keys[name] = idx;
    }
  }

  return this->textures.emplace_back(Handle<Texture>(std::forward<Texture>(texture)));
}

template <>
std::optional<Handle<PbrMaterial>> AssetManager::get<PbrMaterial>(const std::string name) {
  auto found = this->material_keys.find(name);
  if (found != this->material_keys.end()) {
    return this->materials.at(found->second);
  } else {
    return std::nullopt;
  }
}

template <>
std::optional<Handle<Model>> AssetManager::get<Model>(std::string name) {
  auto found = this->mesh_keys.find(name);
  if (found != this->mesh_keys.end()) {
    return this->meshes.at(found->second);
  } else {
    return std::nullopt;
  }
}

template <>
std::optional<Handle<Texture>> AssetManager::get<Texture>(std::string name) {
  auto found = this->texture_keys.find(name);
  if (found != this->texture_keys.end()) {
    return this->textures.at(found->second);
  } else {
    return std::nullopt;
  }
}
