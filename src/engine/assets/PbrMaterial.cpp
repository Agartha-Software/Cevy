/*
** Agartha-Software, 2025
** C++evy
** File description:
** materials implementation
*/

#include "PbrMaterial.hpp"
#include "AssetManager.hpp"
#include "stb_image.h"
#include <iostream>
#include <optional>

using cevy::engine::PbrMaterial;
using cevy::engine::TextureBuilder;
template <typename T>
using Handle = cevy::engine::Handle<T>;

PbrMaterial::PbrMaterial(asset::AssetManager &mngr, const definition &def) : PbrMaterial() {
  TextureBuilder diffuse_builder;
  TextureBuilder specular_builder;
  TextureBuilder metallic_builder;
  TextureBuilder emit_builder;
  TextureBuilder normal_builder;

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

  if (def.metallic.b) {
    metallic_builder.rgb_file_name = def.metallic.b.value();
  }

  if (def.roughness.b) {
    if (def.specular.b) {
      specular_builder.alpha_file_name = def.roughness.b.value();
    } else if (def.metallic.b) {
      metallic_builder.alpha_file_name = def.roughness.b.value();
    } else {
      specular_builder.alpha_file_name = def.roughness.b.value();
    }
  }

  if (def.normal != "") {
    normal_builder.rgb_file_name = def.normal;
  }

  this->diffuse_texture =
      diffuse_builder.good() ? std::make_optional(diffuse_builder.build(mngr)) : std::nullopt;
  this->specular_texture =
      specular_builder.good() ? std::make_optional(specular_builder.build(mngr)) : std::nullopt;
  this->metallic_texture =
      metallic_builder.good() ? std::make_optional(metallic_builder.build(mngr)) : std::nullopt;
  this->specular_texture = this->specular_texture ? this->specular_texture : this->metallic_texture;

  this->emission_texture =
      emit_builder.good() ? std::make_optional(emit_builder.build(mngr)) : std::nullopt;
  this->normal_texture =
      normal_builder.good() ? std::make_optional(normal_builder.build(mngr)) : std::nullopt;

  if (def.metallic.b != "") {
    this->shader = mngr.get<Shader>("gbuffer_pbr");
  } else {
    this->shader = mngr.get<Shader>("gbuffer_generic");
  }

  std::cout << "genereated material:" << std::endl;
  std::cout << "diffuse:" << this->diffuse_texture.has_value() << std::endl;
  std::cout << "specular:" << this->specular_texture.has_value() << std::endl;
  std::cout << "emission:" << this->emission_texture.has_value() << std::endl;
  std::cout << "normal:" << this->normal_texture.has_value() << std::endl;
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
  // new_material.diffuse_texture =
  //     Texture::from_tinyobj(material.diffuse_texname, material.diffuse_texopt);
  // printf("name '%s'\n", new_material.diffuse_texture->file_name.c_str());
  new_material.roughness = 1 / (material.shininess - 1);
  return new_material;
}
