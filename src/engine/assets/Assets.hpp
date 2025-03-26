/*
** Agartha-Software, 2023
** C++evy
** File description:
** Asset
*/

#pragma once

#include "Handle.hpp"
#include "cevy.hpp"
#include <algorithm>
#include <any>
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <unordered_set>
#include <utility>

namespace cevy::engine {
struct ErasedAsset {
  cevy::any asset;
  const bool isShared;
  template<typename A>
  ErasedAsset(typename Handle<A>::shared &&ptr) : asset(std::forward<decltype(ptr)>(ptr)), isShared(true) {};
  template<typename A>
  ErasedAsset(A &&asset) : asset(cevy::make_any<A>(std::forward<A>(asset))), isShared(false) {};
};

template<typename A>
struct AssetsData {
  std::vector<typename Handle<A>::shared> storage;
  std::unordered_set<size_t> recycled;

  bool contains(AssetId id) const {
    return this->storage.at(id)->has_value();
  }
  Handle<A> get(AssetId id) const {
    auto ret = this->storage.at(id);
    if (!ret->has_value()) {
      throw std::out_of_range("Asset Id is uninitialized");
    }
    return Handle<A>(ret, id);
  }
  Handle<A> reserve() {
    if (!this->recycled.empty()) {
      auto id = this->recycled.extract(this->recycled.begin()).value();
      return Handle<A>(this->storage.at(id), AssetId{id});
    } else {
      return Handle<A>(this->storage.emplace_back(std::make_shared<typename Handle<A>::shared::element_type>(std::nullopt)), AssetId{this->storage.size()});
    }
  }
  Handle<A> insert(AssetId id, A&& asset) {
    // within the already allocated storage, so likely recycled
    if (this->storage.size() > id + 1) {
      this->recycled.erase(std::find(this->recycled.begin(), this->recycled.end(), id));
    }
    auto [_, handle] = this->storage.insert(id+1, std::make_shared<typename Handle<A>::shared::element_type>(std::nullopt));
    handle.replace(std::forward<A>(asset));
  }
  Handle<A> add(A &&asset) {
    return this->reserve().replace(std::forward<A>(asset));
  }
};

class ErasedAssetsData;
}

template<>
struct std::hash<cevy::engine::AssetId> {
  size_t operator()(const cevy::engine::AssetId& id) const {
    return std::hash<size_t>()(id.id);
  };
};

namespace cevy::engine {
template <typename A>
class Assets {
  friend class ErasedAssetsData;
  std::shared_ptr<AssetsData<A>> data = std::make_shared<AssetsData<A>>();
  // AssetManager &manager;

  protected:
  void prune() {
    for (auto i = 0; i < this->data->storage.size(); ++i) {
      std::shared_ptr<std::optional<A>> &ptr = this->data->storage.at(i);
      if (ptr.unique() && ptr->has_value()) {
        ptr->emplace(std::nullopt);
        this->data->recycled.insert(i);
      }
    }
  }

  public:

  bool contains(AssetId id) const {
    return this->data->contains(id);
  }
  Handle<A> get(AssetId id) const {
    return this->data->get(id);
  }
  Handle<A> reserve() {
    return this->data->reserve();
  }
  Handle<A> insert(AssetId id, A&& asset) {
    return this->data->insert(id, std::forward<A>(asset));
  }
  Handle<A> add(A &&asset) {
    return this->data->add(std::forward<A>(asset));
  }
};

class ErasedAssetsData {
  cevy::any data;
  std::type_index assetType;
  std::function<ErasedHandle(ErasedAssetsData&)> reserve_callback;
  std::function<ErasedHandle(ErasedAssetsData&, ErasedAsset&&)> add_callback;
  std::function<ErasedHandle(ErasedAssetsData&, const ErasedAssetId&)> get_callback;
  public:
  template<typename A>
  AssetsData<A> &cast() {
    return *std::any_cast<std::shared_ptr<AssetsData<A>>&>(this->data);
  }
  template<typename A>
  ErasedAssetsData(Assets<A> assets) : ErasedAssetsData(assets.data) {}
  template<typename A>
  ErasedAssetsData(std::shared_ptr<AssetsData<A>> &data) : data(cevy::make_any<decltype(data)>(data)), assetType(typeid(A)) {
    this->reserve_callback = [](ErasedAssetsData& data) {
      return ErasedHandle(data.cast<A>().reserve());
    };
    this->add_callback = [](ErasedAssetsData& data, ErasedAsset&& asset) {
      if (asset.isShared) {
        auto &&option = *std::any_cast<typename Handle<A>::shared&&>(std::forward<cevy::any>(asset.asset));
        return ErasedHandle(data.cast<A>().add(std::move<A>(std::forward<std::optional<A>>(option).value())));
      } else {
        return ErasedHandle(data.cast<A>().add(std::any_cast<A&&>(std::forward<cevy::any>(asset.asset))));
      }
    };
    this->get_callback = [](ErasedAssetsData& data, const ErasedAssetId& id) {
      return ErasedHandle(data.cast<A>().get(id.id));
    };
  };

  ErasedHandle reserve() {
    return this->reserve_callback(*this);
  }
  ErasedHandle add(ErasedAsset&& asset) {
    return this->add_callback(*this, std::forward<ErasedAsset>(asset));
  }

  ErasedHandle get(const ErasedAssetId &id) {
    return this->get_callback(*this, id);
  }
};


} // namespace cevy::engine
