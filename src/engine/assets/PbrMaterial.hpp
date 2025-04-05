/*
** Agartha-Software, 2024
** C++evy
** File description:
** Material definition
*/

#pragma once

#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <optional>
#include <string>

#include "Handle.hpp"
#include "Texture.hpp"
#include "engine.hpp"

namespace cevy::engine {
class PbrMaterial {
  public:
  template <typename T, typename V>
  struct pair {
    std::optional<T> a;
    std::optional<V> b;
    pair() : a(std::nullopt), b(std::nullopt) {};
    pair(T &&t) : a(std::forward<T>(t)), b(std::nullopt) {};
    pair(V &&v) : a(std::nullopt), b(std::forward<V>(v)) {};
    pair(T &&t, V &&v) : a(std::forward<T>(t)), b(std::forward<V>(v)) {};
    pair(const T &t, const V &v) : a(t), b(v) {};
    pair(V &&v, T &&t) : a(std::forward<T>(t)), b(std::forward<V>(v)) {};
    pair(const V &v, const T &t) : a(t), b(v) {};
  };

  using color_tex = pair<glm::vec4, std::string>;
  using data_tex = pair<float, std::string>;
  struct definition {
    color_tex diffuse;
    color_tex specular;
    color_tex emit;
    std::string normal = "";
    // additionnal
    data_tex roughness;
    data_tex alpha;
    data_tex metallic;
  };

  public:
  PbrMaterial() { halflambert = true; };

  PbrMaterial(AssetManager &mngr, const definition &def);

  PbrMaterial(glm::vec3 &&diffuse, glm::vec3 &&specular, float roughness)
      : diffuse(diffuse), specular_tint(specular), roughness(roughness) {
    halflambert = true;
  }
  ~PbrMaterial() {};

  PbrMaterial(PbrMaterial &&other) : PbrMaterial() { *this = std::move(other); };

  PbrMaterial &operator=(PbrMaterial &&other) {
    this->emit = other.emit;
    this->ambient = other.ambient;
    this->specular_tint = other.specular_tint;
    this->roughness = other.roughness;
    this->diffuse = other.diffuse;

    this->shader = std::move(other.shader);

    this->diffuse_texture = std::move(other.diffuse_texture);
    this->specular_texture = std::move(other.specular_texture);
    this->emission_texture = std::move(other.emission_texture);
    this->normal_texture = std::move(other.normal_texture);
    return *this;
  }

  static PbrMaterial gold();
  static PbrMaterial from_tinyobj(const tinyobj::material_t &material);

  glm::vec3 emit = {0, 0, 0};
  glm::vec3 ambient = {0, 0, 0};
  glm::vec3 diffuse = {1, 1, 1};
  glm::vec3 specular_tint = {1, 1, 1};
  float roughness = 1;
  bool halflambert : 1;

  std::optional<Handle<Shader>> shader = std::nullopt;

  std::optional<Handle<Texture>> diffuse_texture = std::nullopt;
  std::optional<Handle<Texture>> specular_texture = std::nullopt;
  std::optional<Handle<Texture>> metallic_texture = std::nullopt;
  std::optional<Handle<Texture>> emission_texture = std::nullopt;
  std::optional<Handle<Texture>> normal_texture = std::nullopt;
};
} // namespace cevy::engine
