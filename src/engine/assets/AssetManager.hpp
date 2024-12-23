/*
** Agartha-Software, 2023
** C++evy
** File description:
** Asset Manager
*/

#pragma once

#include "ecs/App.hpp"
#include "Diffuse.hpp"
#include "Model.hpp"
#include "ecs/Plugin.hpp"
#include "ecs/ecs.hpp"

#include <vector>

void init_asset_manager(cevy::ecs::World &w);

namespace cevy::engine {
class AssetManager {
  public:
  // using map = std::unordered_map<std::type_index, std::any>;
  std::vector<cevy::engine::Model> _meshs;
  std::vector<Diffuse> _diffuses;
  // std::vector<ShaderProgram> _shaders;
};

class AssetManagerPlugin : public ecs::Plugin {
  public:
  void build(ecs::App &app);
};
} // namespace cevy::engine
