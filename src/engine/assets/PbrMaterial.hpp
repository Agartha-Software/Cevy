
#pragma once

#include "tinyobj_loader_opt.h"
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <optional>
#include <string>

namespace cevy::engine {
class Texture {
  public:
  std::string file_name;
  uint32_t gl_handle;

  static std::optional<Texture> from_tinyobj(const std::string &file_name,
                                             const tinyobj::texture_option_t &_option);
};

class PbrMaterial {
  public:
  PbrMaterial(){};
  PbrMaterial(glm::vec3 &&diffuse, glm::vec3 &&specular, float exponent)
      : diffuse(diffuse), specular_tint(specular), phong_exponent(exponent) {}
  ~PbrMaterial(){};

  static PbrMaterial gold();
  static PbrMaterial from_tinyobj(const tinyobj::material_t &material);

  glm::vec3 ambiant = {0, 0, 0};
  glm::vec3 diffuse = {0.8, 0.8, 0.8};
  glm::vec3 specular_tint = {1, 1, 1};
  float phong_exponent = 8;

  std::optional<Texture> diffuse_texture = std::optional<Texture>();
};
} // namespace cevy::engine
