/*
** Agartha-Software, 2025
** C++evy
** File description:
** Texture handler
*/

#pragma once

#include <glm/glm.hpp>
#include <optional>

#include "tinyobj_loader_opt.h"

#include "AssetManager.hpp"
#include "ShaderProgram.hpp"

namespace cevy::engine {
using Shader = ShaderProgram;
class Texture {
  bool initted = false;
  std::string file_name;
  GLuint gl_handle = 0;
  friend struct TextureBuilder;

  protected:
  Texture(GLuint gl_texture, std::string file_name) {
    this->gl_handle = gl_texture;
    this->initted = true;
    this->file_name = file_name;
  }

  public:
  enum class Type : int {
    U8_sRGB = 0,
    U8 = 1,
    F16 = 2,
  };

  Texture() = default;
  ~Texture() { deinit(); }

  Texture(Texture &&other) : Texture() { *this = std::move(other); };

  Texture &operator=(Texture &&other) {
    if (this->initted) {
      deinit();
    }
    this->file_name = other.file_name;
    other.file_name = "";
    this->gl_handle = other.gl_handle;
    other.gl_handle = 0;
    return *this;
  }

  GLuint texture_handle() const { return this->gl_handle; };

  void init();

  void deinit();

  static std::optional<Texture> load(const std::string &file_name);

  static std::optional<Texture> from_tinyobj(const std::string &file_name,
                                             const tinyobj::texture_option_t &_option);
};

struct TextureBuilder {
  std::string rgb_file_name = "";
  std::string alpha_file_name = "";
  void *data = nullptr;
  const Texture::Type type;
  int width;
  int height;
  struct {
    bool initted : 1;
    bool has_rgb : 1;
    bool has_alpha : 1;
  } flags;

  inline static constexpr GLenum formats[][2] = {
      {GL_SRGB8_ALPHA8, GL_UNSIGNED_BYTE}, // Texture::Type::U8_sRGB
      {GL_RGB, GL_UNSIGNED_BYTE},          // Texture::Type::U8;
      {GL_RGBA16F, GL_FLOAT},              // Texture::Type::F16
  };

  // AssetManager* manager = nullptr;

  /// must specify an expected type of texture;
  TextureBuilder() = delete;
  TextureBuilder(Texture::Type type) : type(type) {
    flags.initted = false;
    flags.has_alpha = false;
    flags.has_rgb = false;
    this->data = nullptr;
  };
  ~TextureBuilder();
  TextureBuilder(const TextureBuilder &) = delete;
  TextureBuilder(TextureBuilder &&) = delete;

  static Texture from(const glm::vec4 &pixel, int width, int height);
  static Texture from(const glm::vec<4, uint8_t> &pixel, int width, int height);

  int load_rgb();
  int load_alpha();
  int get_alpha(const TextureBuilder &other);

  bool good() const {
    bool is_good = this->data != nullptr || this->rgb_file_name != "" || this->alpha_file_name != "";
    std::cout << "txBuilder:" << this->rgb_file_name << "+" << alpha_file_name << ":rdy?:"
              << is_good
              << std::endl;
    return is_good;
  }
  cevy::engine::Texture build();
  Handle<cevy::engine::Texture> build(asset::AssetManager &manager);
};

} // namespace cevy::engine
