/*
** Agartha-Software, 2023
** C++evy
** File description:
** Asset
*/

#pragma once

#include "AssetManager.hpp"
#include "Handle.hpp"
#include "PbrMaterial.hpp"
#include <utility>

namespace cevy::engine {
template <typename Type>
class Asset {
  AssetManager &manager;

  public:
  Asset(AssetManager &manager) : manager(manager){};

  Handle<Type> load(Type &&asset, const std::string name = "") {
    return this->manager.load(std::forward<Type>(asset), name);
  }
};

} // namespace cevy::engine
