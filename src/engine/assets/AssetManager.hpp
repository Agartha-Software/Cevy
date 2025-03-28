/*
** Agartha-Software, 2023
** C++evy
** File description:
** Asset Manager
*/

#pragma once

#include <algorithm>
#include <functional>
#include <optional>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>
#include <fstream>

#include "App.hpp"
#include "AssetLoader.hpp"
#include "Assets.hpp"
#include "Handle.hpp"
#include "Plugin.hpp"
#include "cevy.hpp"
#include "ecs.hpp"

void init_asset_manager(cevy::ecs::World &w);

namespace cevy::engine {

struct AssetPath {
  std::string source = "";
  std::string path = "";
  std::string label = "";

  AssetPath(const char* cstr) : AssetPath(std::string(cstr)) {};

  AssetPath(const std::string &string) {
    size_t colon = string.find_first_of(':');
    if (colon == std::string::npos) {
      colon = 0;
    } else {
      this->source = string.substr(0, colon);
    }
    size_t pound = string.find_first_of('#', colon);
    if (pound == std::string::npos) {
      pound = string.size();
    }
    this->path = string.substr(colon, pound);
    this->label = string.substr(pound);
  }

  std::vector<std::string> extensions() const {
    std::vector<std::string> ret;
    for (auto it = path.begin(); it < path.end(); ++ it) {
      if (*it == '.' && it + 1 < path.end()) {
        ret.push_back(path.substr(it - path.begin() + 1));
      }
    }
    return ret;
  }

  friend bool operator==(const AssetPath &lhs, const AssetPath &rhs) {
    return lhs.path == rhs.path && lhs.label == rhs.label && lhs.source == rhs.source;
  }
};

}


template<>
struct std::hash<cevy::engine::AssetPath> {
  size_t operator()(const cevy::engine::AssetPath& path) const {
    return hash_pack<std::string, std::string, std::string>()(path.path, path.label, path.source);
  };
};


namespace cevy::engine {
class AssetManager {
  using AssetTypeIndex = std::type_index;
  using LoaderTypeIndex = std::type_index;
  public:
  struct error : public std::runtime_error {
    error(const std::string &msg) : runtime_error(msg) {};
  };

  template<typename A>
  bool ready() const {
    return this->asset_datas.find(typeid(A)) != this->asset_datas.end();
  }

  template <typename A>
  std::optional<Handle<A>> get(const AssetPath& path) {
    if (!this->ready<A>()) {
      throw error("AssetManager::get<" + reflect<A>() + ">: Asset is not ready");
    }
    auto in = this->lookup<A>(path);
    if (in)
      return in;
    // return std::nullopt;
    return this->factory<A>(path);
  }

  template <typename A>
  std::optional<Handle<A>> lookup(const AssetPath& path) {
    if (!this->ready<A>()) {
      throw error("AssetManager::lookup<" + reflect<A>() + ">: Asset is not ready");
    }
    auto found = this->paths.find(path);

    if (found == this->paths.end()) {
      return std::nullopt;
    }
    auto &[_, assetId] = *found;

    return {Handle<A>(this->asset_datas.at(std::type_index(typeid(A))).get(assetId))};
  }

  template <typename A>
  Handle<A> add(A &&asset, const AssetPath& path) {
    if (!this->ready<A>()) {
      throw error("AssetManager::add<" + reflect<A>() + ">: Asset is not ready");
    }
    auto handle = Handle<A>(this->asset_datas.at(std::type_index(typeid(A))).add(std::forward<A>(asset)));

    this->paths.emplace(path, ErasedAssetId(handle));
    // handle.replace(std::forward<A>(asset));
    return handle;
  }

  template<typename A>
  void registerAsset(Assets<A>& assets) {
    auto index = std::type_index(typeid(A));
    this->asset_datas.emplace(index, ErasedAssetsData(assets));
  };

  template<typename Loader>
  void registerLoader(Loader&& loader) {
    auto index = std::type_index(typeid(Loader));
    for (auto &ext: loader.extensions()) {
      this->loader_extensions[ext] = index;
    }
    this->loaders.emplace(index, ErasedAssetLoader(std::forward<Loader>(loader)));
  }

  template<typename A>
  Handle<A> load(const AssetPath& path) {
    if (!this->ready<A>()) {
      throw error("AssetManager::lookup<" + reflect<A>() + ">: Asset is not ready");
    }
    auto index = std::type_index(typeid(A));
    auto handle = this->asset_datas.at(index).reserve();

    std::vector<std::tuple<ErasedAssetLoader*, LoaderTypeIndex>> candidates;

    for (auto &[loader_index, loader] : this->loaders) {
      if (loader.assetType == index)
        candidates.push_back({&loader, loader_index});
    }

    if (candidates.size() == 0) {
      return Handle<A>(handle);
    }

    auto extensions = path.extensions();
    std::sort(extensions.begin(), extensions.end());
    for (auto &[candidate, index] : candidates) {
      for (auto &extension : candidate->extensions()) {
        if (std::binary_search(extensions.begin(), extensions.end(), extension)) {
          return this->loadInternal<A>(path, index);
        }
      }
    }
  }

  template <typename T>
  void add_factory(const AssetPath &path, std::function<T()> &&func) {
    using Func = std::function<T()>;
    this->factories.emplace(
        std::make_pair(path, std::move(cevy::make_any<Func>(std::forward<Func>(func)))));
  }

  protected:
  template <typename Windower>
  friend class Engine;

  template<typename A>
  Handle<A> loadInternal(const AssetPath& path, Handle<A>& handle, const std::type_index &loader) {
    std::basic_ifstream<uint8_t> file(this->sources[""] + path.path);
    std::vector<uint8_t> data(std::istreambuf_iterator<uint8_t>(file), {});

    auto erased = this->loaders.at(loader).load(data, *this);
    return this->asset_datas.at(erased.asset.type()).add(std::move(erased));
  }


  template <typename T>
  std::optional<Handle<T>> factory(const AssetPath &path) {
    using Func = std::function<T()>;
    auto found = this->factories.find(path);
    {
      if (found != this->factories.end()) {
        auto &func = std::any_cast<Func &>(found->second);
        return this->add(std::forward<T>(func()), path);
      }
    }
    return std::nullopt;
  }

  std::unordered_map<std::string, std::string> sources = { {"", "assets/"}};

  std::unordered_map<AssetTypeIndex, ErasedAssetsData> asset_datas;

  std::unordered_map<AssetPath, ErasedAssetId> paths;
  std::unordered_map<LoaderTypeIndex, ErasedAssetLoader> loaders;
  std::unordered_map<std::string, LoaderTypeIndex> loader_extensions;

  // std::unordered_map<std::type_index, std::unordered_map<std::string, size_t>> any_keys;
  // std::unordered_map<std::type_index, cevy::any> anys; // any = std::vector<Asset>

  // std::unordered_map<std::string, size_t> mesh_keys;
  // std::vector<Handle<cevy::engine::Mesh>> meshes;

  // std::unordered_map<std::string, cevy::any> factories;
  std::unordered_map<AssetPath, cevy::any> factories;
  // std::vector<ShaderProgram> _shaders;
};

class AssetManagerPlugin : public ecs::Plugin {
  public:
  void build(ecs::App &app);
};
} // namespace cevy::engine

// template <>
// cevy::engine::Handle<cevy::engine::Mesh>
// cevy::engine::AssetManager::load(cevy::engine::Mesh &&model, std::string name);

// template <>
// std::optional<cevy::engine::Handle<cevy::engine::Mesh>>
// cevy::engine::AssetManager::get<cevy::engine::Mesh>(std::string name);
