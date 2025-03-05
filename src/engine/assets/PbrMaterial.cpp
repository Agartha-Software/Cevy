/*
** Agartha-Software, 2025
** C++evy
** File description:
** materials implementation
*/

#include <functional>
#include "glx.hpp"

#include "AssetManager.hpp"
#include "PbrMaterial.hpp"
#include "cevy.hpp"
#include "stb_image.h"
#include <cstring>
#include <iostream>
#include <optional>
#include <stdexcept>

using cevy::engine::PbrMaterial;
using cevy::engine::Texture;
using cevy::engine::TextureBuilder;
template <typename T>
using Handle = cevy::engine::Handle<T>;

std::optional<Texture> Texture::from_tinyobj(const std::string &file_name,
                                             const tinyobj::texture_option_t & /* _option */) {
  Texture new_texture;
  new_texture.file_name = file_name;

  if (file_name != "") {
    int width, height, nrChannels;
    std::string path = "./assets/" + file_name;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *image_data = stbi_load(&path[0], &width, &height, &nrChannels, 0);

    if (!image_data) {
      std::cerr << "Error: from_tinyobj: Failed to load [" << path << "] texture" << std::endl;
      stbi_image_free(image_data);
      return std::optional<Texture>();
    }

    glGenTextures(1, &new_texture.gl_handle);
    glBindTexture(GL_TEXTURE_2D, new_texture.gl_handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 image_data); // HERE
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(image_data);

    return new_texture;
  }
  return std::optional<Texture>();
}

PbrMaterial::PbrMaterial(AssetManager &mngr, const definition &def) : PbrMaterial() {
  TextureBuilder diffuse_builder;
  TextureBuilder specular_builder;
  TextureBuilder emit_builder;

  if (def.diffuse.a) {
    this->diffuse = def.diffuse.a.value();
  }
  if (def.diffuse.b) {
    diffuse_builder.rgb_file_name = def.diffuse.b.value();
  }
  if (def.specular.b) {
    specular_builder.rgb_file_name = def.specular.b.value();
  }
  if (def.emit.b) {
    emit_builder.rgb_file_name = def.emit.b.value();
  }

  if (def.alpha.b) {
    diffuse_builder.alpha_file_name = def.alpha.b.value();
  }

  if (def.roughness.b) {
    specular_builder.alpha_file_name = def.roughness.b.value();
  }

  this->diffuse_texture = diffuse_builder.build(mngr);
  this->specular_texture = specular_builder.build(mngr);
  this->emission_texture = emit_builder.build(mngr);
}

PbrMaterial PbrMaterial::gold() {
  return PbrMaterial({0.003, 0.00225, 0.001}, {0.6, 0.4, 0.1}, 10);
}

PbrMaterial PbrMaterial::from_tinyobj(const tinyobj::material_t &material) {
  PbrMaterial new_material;

  printf("name %s\n", material.name.c_str());
  new_material.ambient = {material.ambient[0], material.ambient[1], material.ambient[2]};
  printf("ambient %f %f %f \n", new_material.ambient.x, new_material.ambient.y,
         new_material.ambient.z);
  new_material.diffuse = {material.diffuse[0], material.diffuse[1], material.diffuse[2]};
  printf("diffuse %f %f %f \n", new_material.diffuse.x, new_material.diffuse.y,
         new_material.diffuse.z);
  new_material.specular_tint = {material.specular[0], material.specular[1], material.specular[2]};
  printf("spec %f %f %f \n", new_material.specular_tint.x, new_material.specular_tint.y,
         new_material.specular_tint.z);
  new_material.diffuse_texture =
      Texture::from_tinyobj(material.diffuse_texname, material.diffuse_texopt);
  // printf("name '%s'\n", new_material.diffuse_texture->file_name.c_str());
  new_material.phong_exponent = material.shininess;
  return new_material;
}

TextureBuilder::~TextureBuilder() {
  if (this->data)
    stbi_image_free(data);
  data = nullptr;
}

Texture TextureBuilder::from(const glm::vec4u8 &pixel, int width, int height) {
  TextureBuilder builder;
  builder.width = width;
  builder.height = height;
  builder.data = malloc(width * height * 4 * sizeof(uint8_t));
  builder.type = Texture::Type::U8;

  for (int x = 0; x < width; ++x)
    for (int y = 0; y < height; ++y) {
      static_cast<glm::vec4u8 *>(builder.data)[x + y * width] = pixel;
    }

  // std::fill(reinterpret_cast<glm::vec<4, uint8_t> *>(builder.data),
  //           reinterpret_cast<glm::vec<4, uint8_t> *>(builder.data) + width * height, pixel);
  return builder.build().value();
}

Texture TextureBuilder::from(const glm::vec4 &pixel, int width, int height) {
  TextureBuilder builder;
  builder.width = width;
  builder.height = height;
  builder.data = malloc(width * height * 4 * sizeof(float));
  builder.type = Texture::Type::F16;

  for (int x = 0; x < width; ++x)
    for (int y = 0; y < height; ++y) {
      static_cast<glm::vec4 *>(builder.data)[x + y * width] = pixel;
    }

  // std::fill(reinterpret_cast<glm::vec4 *>(builder.data),
  //           reinterpret_cast<glm::vec4 *>(builder.data) + width * height, pixel);
  return builder.build().value();
}

static uint8_t *normalize_image_data(uint8_t *image_data, int width, int height, int nrChannels,
                                     uint8_t default_value = 0) {
  using DataType = uint8_t;
  uint8_t *new_data = static_cast<uint8_t *>(malloc(width * height * sizeof(DataType)));

  for (int x = 0; x < width; ++x) {
    for (int y = 0; y < height; ++y) {
      int i = 0;
      for (i = 0; i < nrChannels; ++i) {
        new_data[(x + y * width) * 4 + i] = image_data[(x + y * width) * nrChannels + i];
      }
      for (; i < 4; ++i) {
        new_data[(x + y * width) * 4 + i] = default_value;
      }
    }
  }
  return new_data;
}

static void splice_image_data(uint8_t *rgb_data, const uint8_t *alpha_data, int width, int height,
                              int alpha_channels, int alpha_alpha_chan) {
  for (int x = 0; x < width; ++x) {
    for (int y = 0; y < height; ++y) {
      rgb_data[(x + y * width) * 4 + 3] =
          alpha_data[(x + y * width) * alpha_channels + alpha_alpha_chan];
    }
  }
}

int TextureBuilder::load_rgb() {
  int width = 0;
  int height = 0;
  int nrChannels = 0;
  stbi_set_flip_vertically_on_load(true);
  uint8_t *image_data = stbi_load(this->rgb_file_name.c_str(), &width, &height, &nrChannels, 0);

  if (!image_data) {
    std::cerr << "Error: load_rgb: Failed to load [" << this->rgb_file_name << "] texture"
              << std::endl;
    return -1;
  }

  this->width = width;
  this->height = height;

  if (nrChannels == 4) {
    this->data = image_data;
    return 0;
  }

  uint8_t *new_data = normalize_image_data(image_data, width, height, nrChannels, 255);
  stbi_image_free(image_data);
  this->data = new_data;
  this->type = Texture::Type::U8_sRGB;
  return 0;
}

int TextureBuilder::load_alpha() {
  using DataType = uint8_t;
  int width = 0;
  int height = 0;
  int nrChannels = 0;

  stbi_set_flip_vertically_on_load(true);
  uint8_t *alpha_data = stbi_load(this->rgb_file_name.c_str(), &width, &height, &nrChannels, 0);

  if (!alpha_data) {
    std::cerr << "Error: load_alpha: Failed to load [" << this->rgb_file_name << "] texture"
              << std::endl;
    return -1;
  }

  if (this->width != width || this->width != height) {
    stbi_image_free(alpha_data);
    throw std::runtime_error("TextureBuilder::load_alpha: '" + this->alpha_file_name +
                             "' Image texture has differently sized alpha");
  }

  if (!this->data) {
    this->data = static_cast<uint8_t *>(malloc(width * height * sizeof(DataType)));
    std::memset(this->data, 255, width * height * sizeof(DataType));
    this->type = Texture::Type::U8_sRGB;
  }

  splice_image_data(static_cast<uint8_t *>(this->data), alpha_data, this->width, this->height,
                    nrChannels, 1);
  return 0;
}

int TextureBuilder::get_alpha(const TextureBuilder &other) {
  using DataType = uint8_t;
  if (this->width != other.width || this->width != other.height) {
    throw std::runtime_error("TextureBuilder::get_alpha: '" + other.alpha_file_name +
                             "' Image texture has differently sized alpha");
  }

  if (!this->data) {
    this->data = static_cast<uint8_t *>(malloc(width * height * sizeof(DataType)));
    std::memset(this->data, 255, width * height * sizeof(DataType));
    this->type = Texture::Type::U8_sRGB;
  }

  splice_image_data(static_cast<uint8_t *>(this->data), static_cast<uint8_t *>(other.data),
                    this->width, this->height, 4, 3);
  return 0;
}

std::optional<Texture> TextureBuilder::build() {
  if (this->rgb_file_name != "") {
    if (this->load_rgb()) {
      return std::nullopt;
    }
  }
  if (this->alpha_file_name != "") {
    if (this->load_alpha()) {
      return std::nullopt;
    }
  }

  std::string name_full = this->rgb_file_name;
  if (this->alpha_file_name != "") {
    name_full += this->alpha_file_name;
  }

  if (this->data) {
    uint32_t texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, TextureBuilder::formats[int(this->type)][0], this->width,
                 this->height, 0, GL_RGBA, TextureBuilder::formats[int(this->type)][1], this->data);

    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA,
    // GL_UNSIGNED_BYTE,
    //              this->data);
    glGenerateMipmap(GL_TEXTURE_2D);

    return Texture(texture, name_full);
  }
  return std::nullopt;
}

std::optional<Handle<Texture>> TextureBuilder::build(AssetManager &manager) {
  std::string name_full = this->rgb_file_name;
  if (this->alpha_file_name != "") {
    name_full += this->alpha_file_name;
  }

  auto o_tex = manager.get<Texture>();

  if (o_tex) {
    return o_tex;
  }

  if (this->rgb_file_name != "") {
    if (this->load_rgb()) {
      return std::nullopt;
    }
  }
  if (this->alpha_file_name != "") {
    if (this->load_alpha()) {
      return std::nullopt;
    }
  }

  if (this->data) {
    uint32_t texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 this->data);
    glGenerateMipmap(GL_TEXTURE_2D);

    return manager.load(Texture(texture, name_full), name_full);
  }
  return std::nullopt;
}

void cevy::engine::Texture::init() {}

void cevy::engine::Texture::deinit() {}
