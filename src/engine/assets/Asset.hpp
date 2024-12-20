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
  Asset<Diffuse>(AssetManager &ref) : _ref(ref){};

  Handle<Diffuse> load(const std::string &path) {
    _ref._diffuses.push_back(Diffuse(LoadTexture(path.c_str())));
    return Handle<Diffuse>(_ref._diffuses[_ref._diffuses.size() - 1]);
  }
};

template <>
class Asset<cevy::engine::Mesh> {
  using Type = Mesh;

  AssetManager &_ref;

  public:
  Asset<Mesh>(AssetManager &ref) : _ref(ref){};

  Handle<Mesh> load(const std::string &path) {
    _ref._meshs.push_back(Mesh(LoadModel(path.c_str())));
    return Handle<Mesh>(_ref._meshs[_ref._meshs.size() - 1]);
  }
};

} // namespace cevy::engine
