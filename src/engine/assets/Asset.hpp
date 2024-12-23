/*
** Agartha-Software, 2023
** C++evy
** File description:
** Asset
*/

#pragma once

#include "AssetManager.hpp"
#include "Handle.hpp"

namespace cevy::engine {
template <typename Type>
class Asset {};

template <>
class Asset<Diffuse> {
  using Type = Diffuse;

  AssetManager &_ref;

  public:
  Asset(AssetManager &ref) : _ref(ref){};

  Handle<Diffuse> load(const std::string &path) {
    // _ref._diffuses.push_back(Diffuse(LoadTexture(path.c_str())));
    _ref._diffuses.push_back(Diffuse());
    return Handle<Diffuse>(_ref._diffuses[_ref._diffuses.size() - 1]);
  }
};

template <>
class Asset<cevy::engine::Model> {
  using Type = Model;

  AssetManager &_ref;

  public:
  Asset(AssetManager &ref) : _ref(ref){};

  Handle<Model> load(Model&& model) {
    _ref._meshs.push_back(model);
    return Handle<Model>(_ref._meshs[_ref._meshs.size() - 1]);
  }
};

} // namespace cevy::engine
