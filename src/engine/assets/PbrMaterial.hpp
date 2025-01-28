
#pragma once

#include "Handle.hpp"
#include "engine.hpp"
#include "tinyobj_loader_opt.h"
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <optional>
#include <string>

namespace cevy::engine {
class Texture {
  bool initted = false;
  std::string file_name;
  uint32_t gl_handle;
  friend struct TextureBuilder;

  protected:
  Texture(uint gl_texture, std::string file_name) {
    this->gl_handle = gl_texture;
    this->initted = true;
    this->file_name = file_name;
  }

  public:
  Texture() = default;
  ~Texture() {
    deinit();
  }


  Texture(Texture &&other) : Texture() {
    *this = std::move(other);
  };

  Texture& operator=(Texture &&other) {
    if (this->initted) {
      deinit();
    }
    this->file_name = other.file_name;
    other.file_name = "";
    this->gl_handle = other.gl_handle;
    other.gl_handle = 0;
    return *this;
  }

  uint texture_handle() const {return this->gl_handle; };

  void init();

  void deinit();

  static std::optional<Texture> load(const std::string& file_name);

  static std::optional<Texture> from_tinyobj(const std::string &file_name,
                                             const tinyobj::texture_option_t &_option);
};

struct TextureBuilder {
  std::string rgb_file_name;
  std::string alpha_file_name;
  uint8_t *data = nullptr;
  int width;
  int height;
  struct {
    bool initted : 1;
    bool has_rgb : 1;
    bool has_alpha : 1;
  } flags;

  // AssetManager* manager = nullptr;

  TextureBuilder() { flags.initted = false; flags.has_alpha = false; flags.has_rgb = false; };
  ~TextureBuilder();
  TextureBuilder(const TextureBuilder&) = delete;
  TextureBuilder(TextureBuilder&&) = delete;

  static Texture from(const glm::vec4& pixel, int width, int height);

  int load_rgb();
  int load_alpha();
  int get_alpha(const TextureBuilder& other);
  std::optional<cevy::engine::Texture> build();
  std::optional<Handle<cevy::engine::Texture>> build(AssetManager& manager);
};

class PbrMaterial {
public:
  template<typename T, typename V>
  struct pair {
    std::optional<T> a;
    std::optional<V> b;
    pair() : a(std::nullopt), b(std::nullopt) {};
    pair(T&& t) : a(std::forward<T>(t)), b(std::nullopt) {};
    pair(V&& v) : a(std::nullopt), b(std::forward<V>(v)) {};
    pair(T&& t, V&& v) : a(std::forward<T>(t)), b(std::forward<V>(v)) {};
    pair(V&& v, T&& t) : a(std::forward<T>(t)), b(std::forward<V>(v)) {};
  };

  using color_tex = pair<glm::vec4, std::string>;
  using data_tex = pair<float, std::string>;
  struct definition {
    color_tex diffuse;
    color_tex specular;
    color_tex emit;
    // additionnal
    data_tex roughness;
    data_tex alpha;
  };

  public:
  PbrMaterial(){
    halflambert = true;
  };

  PbrMaterial(AssetManager &mngr, const definition &def);

  PbrMaterial(glm::vec3 &&diffuse, glm::vec3 &&specular, float exponent)
      : diffuse(diffuse), specular_tint(specular), phong_exponent(exponent) {
        halflambert = true;
      }
  ~PbrMaterial(){};

  PbrMaterial(PbrMaterial &&other) : PbrMaterial() {
    *this = std::move(other);
  };

  PbrMaterial& operator=(PbrMaterial &&other) {
    this->emit = other.emit;
    this->ambient = other.ambient;
    this->specular_tint = other.specular_tint;
    this->phong_exponent = other.phong_exponent;
    return *this;
  }

  static PbrMaterial gold();
  static PbrMaterial from_tinyobj(const tinyobj::material_t &material);

  glm::vec3 emit = {0, 0, 0};
  glm::vec3 ambient = {0, 0, 0};
  glm::vec3 diffuse = {0.8, 0.8, 0.8};
  glm::vec3 specular_tint = {1, 1, 1};
  float phong_exponent = 8;
  bool halflambert: 1;

  std::optional<Handle<Texture>> diffuse_texture = std::nullopt;
  std::optional<Handle<Texture>> specular_texture = std::nullopt;
  std::optional<Handle<Texture>> emission_texture = std::nullopt;
};
} // namespace cevy::engine
