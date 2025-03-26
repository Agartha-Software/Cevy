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

namespace cevy::engine {
class AssetManager;

class LoadContext {
  protected:
  friend class AssetManager;
  LoadContext(AssetManager &manager) : manager(manager) {};
  AssetManager &manager;

  public:
};
template <typename A>
class AssetLoader {
  public:
  AssetLoader() {};
  using Asset = A;
  // using Settings = S;

  // template<typename T = Asset>
  // auto load(const std::vector<uint8_t> &data, const LoadContext &context) const ->
  // std::enable_if_t<std::is_default_constructible_v<Settings>, T> {
  //   load(data, context, Settings());
  // }

  virtual const std::vector<std::string> &extensions() const noexcept = 0;

  virtual Asset load(const std::vector<uint8_t> &data, const LoadContext &context) const = 0;

  // protected:
  // ErasedAsset loadErased(const std::vector<uint8_t> &data,  const LoadContext &context) const
  // override final {
  //   ErasedAsset {cevy::make_any<Asset>(this->load(data, context), context),
  //   std::type_info(typeid(Asset))};
  // }
  // virtual const std::vector<std::string> extensions() const override = 0;
};

class ErasedAssetLoader {
  protected:
  template <typename A>
  ErasedAssetLoader(AssetLoader<A> &&loader)
      : loader(loader), assetType(std::type_info(typeid(A))) {
    this->load_callback = [](ErasedAssetLoader &erased, const std::vector<uint8_t> &data,
                             const LoadContext &context) {
      return ErasedAsset(std::make_shared(
          std::any_cast<const AssetLoader<A> &>(erased.loader).load(data, context)));
    };
    this->extensions_callback = [](ErasedAssetLoader &erased) {
      return std::any_cast<const AssetLoader<A> &>(erased.loader).extensions();
    };
  };

  public:
  cevy::any loader;
  std::type_index assetType;
  std::function<ErasedAsset(const ErasedAssetLoader &, const std::vector<uint8_t> &data,
                            const LoadContext &context)>
      load_callback;
  std::function<const std::vector<std::string> &(const ErasedAssetLoader &)> extensions_callback;

  const std::vector<std::string> &extensions() const noexcept {
    return this->extensions_callback(*this);
  }

  ErasedAsset load(const std::vector<uint8_t> &data, const LoadContext &context) const {
    return this->load_callback(*this, data, context);
  }
};

} // namespace cevy::engine
