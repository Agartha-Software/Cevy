/*
** Agartha-Software, 2023
** C++evy
** File description:
** Asset Loader definition
*/

#pragma once

#include "cevy.hpp"
#include <any>
#include <cstdint>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <vector>

#include "Assets.hpp"


namespace cevy::engine::asset {
class AssetManager;
class LoadContext;
struct LoadedAsset;

// template <typename A>
class AssetLoader {
  protected:
  using Asset = void; // default assetloaders that don't specify a type to void for SFINAE
  public:
  AssetLoader() {};

  virtual ~AssetLoader() = default;

  virtual const std::vector<std::string> &extensions() const noexcept = 0;

  virtual const std::type_index &assetType() const noexcept = 0;

  virtual LoadedAsset load(const std::vector<std::byte> &data, LoadContext context) const = 0;
};
} // namespace cevy::engine
