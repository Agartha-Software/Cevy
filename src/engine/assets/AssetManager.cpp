/*
** Agartha-Software, 2023
** C++evy
** File description:
** Asset Manager
*/

#include "AssetManager.hpp"
#include "Asset.hpp"
#include "Diffuse.hpp"
#include "engine/Engine.hpp"

void init_asset_manager(cevy::ecs::World &w) {
  w.insert_resource(cevy::engine::AssetManager());
  auto val = w.get_resource<cevy::engine::AssetManager>();
  if (val) {
    w.insert_resource(cevy::engine::Asset<cevy::engine::Model>(val->get()));
    w.insert_resource(cevy::engine::Asset<cevy::engine::Diffuse>(val->get()));
  }
}

void cevy::engine::AssetManagerPlugin::build(ecs::App &app) {
  app.add_systems<PostStartupRenderStage>(init_asset_manager);
  app.init_component<Handle<cevy::engine::Model>>();
  app.init_component<Handle<cevy::engine::Diffuse>>();
}
