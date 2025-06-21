/*
** Agartha-Software, 2025
** C++evy
** File description:
** textures implementation
*/

#ifndef DEBUG
#define STBI_MALLOC(sz) ((void *)(new char[sz]))
#define STBI_REALLOC_SIZED(p, oldsz, newsz)                                                        \
  (size_t(newsz) <= size_t(oldsz) ? (void *)(p)                                                    \
                                  : [](void *lp, size_t loldsz, size_t lnewsz) -> void * {         \
    char *n = new char[lnewsz];                                                                    \
    memcpy(n, lp, loldsz);                                                                         \
    delete[] (char *)lp;                                                                           \
    return (void *)n;                                                                              \
  }(p, oldsz, newsz))
#define STBI_FREE(p) (delete[] (char *)(p))
#endif // DEBUG

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include <cstring>

#include "Texture.hpp"
#include "engine.hpp"

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

TextureBuilder::~TextureBuilder() {
  if (this->data)
    stbi_image_free(this->data);
  this->data = nullptr;
}

Texture TextureBuilder::from(const glm::vec4u8 &pixel, int width, int height) {
  TextureBuilder builder(Texture::Type::U8);
  builder.width = width;
  builder.height = height;
  builder.data = malloc(width * height * 4 * sizeof(uint8_t));

  for (int x = 0; x < width; ++x)
    for (int y = 0; y < height; ++y) {
      static_cast<glm::vec4u8 *>(builder.data)[x + y * width] = pixel;
    }

  // std::fill(reinterpret_cast<glm::vec<4, uint8_t> *>(builder.data),
  //           reinterpret_cast<glm::vec<4, uint8_t> *>(builder.data) + width * height, pixel);
  return builder.build();
}

Texture TextureBuilder::from(const glm::vec4 &pixel, int width, int height) {
  TextureBuilder builder(Texture::Type::F16);
  builder.width = width;
  builder.height = height;
  builder.data = malloc(width * height * 4 * sizeof(float));

  for (int x = 0; x < width; ++x)
    for (int y = 0; y < height; ++y) {
      static_cast<glm::vec4 *>(builder.data)[x + y * width] = pixel;
    }

  // std::fill(reinterpret_cast<glm::vec4 *>(builder.data),
  //           reinterpret_cast<glm::vec4 *>(builder.data) + width * height, pixel);
  return builder.build();
}

static uint8_t *normalize_image_data(uint8_t *image_data, int width, int height, int nrChannels,
                                     uint8_t default_value = 0) {
  using DataType = uint8_t;
  uint8_t *new_data = static_cast<uint8_t *>(malloc(width * height * sizeof(DataType) * 4));

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
  // this->type = Texture::Type::U8_sRGB;
  // this->type = Texture::Type::U8;
  return 0;
}

int TextureBuilder::load_alpha() {
  using DataType = uint8_t;
  int width = 0;
  int height = 0;
  int nrChannels = 0;

  stbi_set_flip_vertically_on_load(true);
  uint8_t *alpha_data = stbi_load(this->alpha_file_name.c_str(), &width, &height, &nrChannels, 0);

  if (!alpha_data) {
    std::cerr << "Error: load_alpha: Failed to load [" << this->alpha_file_name << "] texture"
              << std::endl;
    return -1;
  }

  if (!this->data) {
    this->width = width;
    this->height = height;
    this->data = static_cast<uint8_t *>(malloc(width * height * sizeof(DataType) * 4));
    std::memset(this->data, 255, width * height * 4);
  }

  if (this->width != width || this->width != height) {
    stbi_image_free(alpha_data);
    throw std::runtime_error("TextureBuilder::load_alpha: '" + this->alpha_file_name +
                             "' Image texture has differently sized alpha");
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
    this->data = static_cast<uint8_t *>(malloc(width * height * sizeof(DataType) * 4));
    std::memset(this->data, 255, width * height * 4);
  }

  splice_image_data(static_cast<uint8_t *>(this->data), static_cast<uint8_t *>(other.data),
                    this->width, this->height, 4, 3);
  return 0;
}

Texture TextureBuilder::build() {
  std::string name_full = this->rgb_file_name;
  if (this->alpha_file_name != "") {
    name_full += this->alpha_file_name;
  }

  if (this->rgb_file_name != "") {
    if (this->load_rgb()) {
      throw std::runtime_error("TextureBuilder failed at load_rgb:" + name_full);
    }
  }
  if (this->alpha_file_name != "") {
    if (this->load_alpha()) {
      throw std::runtime_error("TextureBuilder failed at load_alpha:" + name_full);
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

    glTexImage2D(GL_TEXTURE_2D, 0, TextureBuilder::formats[int(this->type)][0], this->width,
                 this->height, 0, GL_RGBA, TextureBuilder::formats[int(this->type)][1], this->data);

    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA,
    // GL_UNSIGNED_BYTE,
    //              this->data);
    glGenerateMipmap(GL_TEXTURE_2D);

    return Texture(texture, name_full);
  }
  throw std::runtime_error("TextureBuilder failed at this->data (without load):" + name_full);
}

Handle<Texture> TextureBuilder::build(asset::AssetManager &manager) {
  std::string name_full = this->rgb_file_name;
  if (this->alpha_file_name != "") {
    name_full += "_" + this->alpha_file_name;
  }

  auto o_tex = manager.get<Texture>(name_full);

  if (o_tex) {
    return o_tex.value();
  }

  if (this->rgb_file_name != "") {
    if (this->load_rgb()) {
      throw std::runtime_error("TextureBuilder failed at load_rgb:" + name_full);
    }
  }
  if (this->alpha_file_name != "") {
    if (this->load_alpha()) {
      throw std::runtime_error("TextureBuilder failed at load_alpha:" + name_full);
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

    // glTexImage2D(GL_TEXTURE_2D, 0, TextureBuilder::formats[int(this->type)][0], this->width,
    //              this->height, 0, GL_RGBA, TextureBuilder::formats[int(this->type)][1],
    //              this->data);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 this->data);

    // glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, this->width, this->height, 0, GL_RGBA,
    // GL_UNSIGNED_BYTE,
    //         this->data);

    glGenerateMipmap(GL_TEXTURE_2D);

    std::cout << "successfully generated '" << name_full << "'" << std::endl;
    return manager.add(Texture(texture, name_full), name_full);
  }
  throw std::runtime_error("TextureBuilder failed at this->data (with load):" + name_full);
}

void cevy::engine::Texture::init() {}

void cevy::engine::Texture::deinit() {}
