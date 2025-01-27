/*
** Agartha-Software, 2023
** C++evy
** File description:
** Asset Manager
*/

#include "AssetManager.hpp"
#include "Asset.hpp"
#include "PbrMaterial.hpp"
#include "engine/Engine.hpp"

void init_asset_manager(cevy::ecs::World &w) {
  w.init_resource(cevy::engine::AssetManager());
  auto asset_manager = w.get_resource<cevy::engine::AssetManager>();
  if (asset_manager) {
    w.insert_resource(cevy::engine::Asset<cevy::engine::Model>(asset_manager->get()));
    w.insert_resource(cevy::engine::Asset<cevy::engine::PbrMaterial>(asset_manager->get()));
  }
}

void cevy::engine::AssetManagerPlugin::build(ecs::App &app) {
  app.add_systems<PostStartupRenderStage>(init_asset_manager);
  app.init_component<Handle<cevy::engine::Model>>();
  app.init_component<Handle<cevy::engine::PbrMaterial>>();
}
