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

  AssetPath(const std::string &string) : AssetPath(std::string_view(string)) {}
  AssetPath(const std::string_view &string) {
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

  AssetPath labeled(const std::string& label) const {
    AssetPath ret = *this;
    ret.label = label;
    return ret;
  }

  friend bool operator==(const AssetPath &lhs, const AssetPath &rhs) {
    return lhs.path == rhs.path && lhs.label == rhs.label && lhs.source == rhs.source;
  }

  std::string pretty() const {
    auto source_pfx = this->source + (this->source != "" ? ":" : "");
    auto label_sfx = (this->label != "" ? "#" : "") + this->label;
    return source_pfx + this->path + label_sfx;
  }
};

} // namespace cevy::engine

template<>
struct std::hash<cevy::engine::AssetPath> {
  size_t operator()(const cevy::engine::AssetPath& path) const {
    return hash_pack<std::string, std::string, std::string>()(path.path, path.label, path.source);
  };
};


namespace cevy::engine::asset {
struct LoadedAsset {
  protected:
  friend class AssetManager;
  friend class LoadContext;
  AssetPath path;
  ErasedHandle handle;
  LoadedAsset(const AssetPath &path, const ErasedHandle &handle) : path(path), handle(handle) {};
};

class LoadContext {
  protected:
  friend class AssetManager;
  LoadContext(AssetManager &manager, const AssetPath& path, ErasedHandle handle) : manager(manager), path(path), handle(std::move(handle)) {};
  AssetManager &manager;
  AssetPath path;
  ErasedHandle handle;

  public:
  const std::type_index &assetType() const { return handle.type; }
  AssetPath &getPath() { return this->path; }
  template<typename A>
  Handle<A> addLabeledAsset(const std::string &label, A&& asset);
  template<typename A>
  LoadedAsset finish(A&& asset);


  template<typename A>
  Handle<A> getAsset(const AssetPath &path);

  template<typename A>
  Handle<A> load(const AssetPath &path);
};

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
    static_assert(typeid(typename Loader::Asset) != typeid(void), "Loader must specify a member decltype Asset!");
    auto loader_index = std::type_index(typeid(Loader));
    auto asset_index = std::type_index(typeid(typename Loader::Asset));

    for (auto &ext: loader.extensions()) {
      this->loaderExtensions[ext] = loader_index;
    }
    this->loaderTypeLookup[asset_index] = loader_index;
    this->loaderType[loader_index] = asset_index;
    this->loaders.emplace(loader_index, ErasedAssetLoader(std::forward<Loader>(loader)));
  }

  template<typename A>
  Handle<A> load(const AssetPath& path) {
    if (!this->ready<A>()) {
      throw error("AssetManager::lookup<" + reflect<A>() + ">: Asset is not ready");
    }
    auto loader_index = this->find_loader(path, typeid(A));
    if (loader_index == typeid(void)) {
      throw std::runtime_error("AssetManager::load: no loader for extensions '" + path.path.substr(std::max(0l, (signed long)(path.path.find('.')))) + "' with type '" + reflect<A>() + "'");
    }
    auto main_asset_index = this->loaderType.at(loader_index);

    if (auto found = this->paths.find(path); found != this->paths.end()) {
      return this->asset_datas.at(main_asset_index).cast<A>().get(found->second.id);
    }

    auto main_handle = this->asset_datas.at(main_asset_index).reserve();

    this->loadInternal(path, main_handle, loader_index);
    const auto &erased_id = this->paths.at(path);
    if (erased_id.type != typeid(A)) {
      throw std::runtime_error("AssetManager::load: Asset [" + path.pretty() + "] is not of type '" + reflect<A>() + "' (is of type '" + erased_id.type.name() + "')");
    }
    return this->asset_datas.at(erased_id.type).get(erased_id).cast<A>();
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

  LoaderTypeIndex find_loader(const AssetPath path, const AssetTypeIndex &asset_type) {
    auto extensions = path.extensions();

    std::vector<std::tuple<AssetLoader*, LoaderTypeIndex>> candidates;

    for (auto &[ext, loader_index] : this->loaderExtensions) {
      if (std::find(extensions.begin(), extensions.end(), ext) != extensions.end()) {
        // unlabeled assets type must match the main asset type
        if (path.label != "" || this->loaderType.at(loader_index) == asset_type) {
          return loader_index;
        }
      }
    }

    return typeid(void);
  }

  void loadInternal(const AssetPath& path, ErasedHandle& handle, const std::type_index &loader) {
    std::basic_ifstream<std::byte> file(this->sources[""] + path.path);
    std::vector<std::byte> data(std::istreambuf_iterator<std::byte>(file), {});

    auto loaded = this->loaders.at(loader)->load(data, LoadContext(*this, path, handle));
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
  std::unordered_map<LoaderTypeIndex, std::unique_ptr<AssetLoader>> loaders;
  std::unordered_map<std::string, LoaderTypeIndex> loaderExtensions;
  std::unordered_map<AssetTypeIndex, LoaderTypeIndex> loaderTypeLookup;
  std::unordered_map<LoaderTypeIndex, AssetTypeIndex> loaderType;

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

template<typename A>
inline Handle<A> cevy::engine::asset::LoadContext::addLabeledAsset(const std::string &label, A&& asset) {
  this->manager.add(std::forward<A>(asset), this->path.labeled(label));
}

template<typename A>
inline LoadedAsset cevy::engine::asset::LoadContext::finish(A&& asset) {
  this->handle.cast<A>().replace(std::move(asset));
  return LoadedAsset(this->path, std::move(this->handle));
}

template<typename A>
inline Handle<A> cevy::engine::asset::LoadContext::getAsset(const AssetPath &path) {
  return this->manager.get<A>(path);
}

template<typename A>
inline Handle<A> cevy::engine::asset::LoadContext::load(const AssetPath &path) {
  return this->manager.load<A>(path);
}

} // namespace cevy::engine

// template <>
// cevy::engine::Handle<cevy::engine::Mesh>
// cevy::engine::AssetManager::load(cevy::engine::Mesh &&model, std::string name);

// template <>
// std::optional<cevy::engine::Handle<cevy::engine::Mesh>>
// cevy::engine::AssetManager::get<cevy::engine::Mesh>(std::string name);
