/*
** Agartha-Software, 2025
** C++evy
** File description:
** materials implementation
*/

#include "PbrMaterial.hpp"
#include "AssetManager.hpp"
#include "Handle.hpp"
#include "stb_image.h"
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>

using cevy::engine::PbrMaterial;
using cevy::engine::TextureBuilder;
template <typename T>
using Handle = cevy::engine::Handle<T>;

PbrMaterial::PbrMaterial(asset::AssetManager &mngr, const definition &def) : PbrMaterial() {
  TextureBuilder diffuse_builder(Texture::Type::U8_sRGB);
  TextureBuilder specular_builder(Texture::Type::U8_sRGB);
  TextureBuilder metallic_builder(Texture::Type::U8);
  TextureBuilder emit_builder(Texture::Type::U8_sRGB);
  TextureBuilder normal_builder(Texture::Type::U8);

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


PbrMaterial::PbrMaterial(const definition &def) : PbrMaterial() {
  TextureBuilder diffuse_builder(Texture::Type::U8_sRGB);
  TextureBuilder specular_builder(Texture::Type::U8_sRGB);
  TextureBuilder metallic_builder(Texture::Type::U8);
  TextureBuilder emit_builder(Texture::Type::U8_sRGB);
  TextureBuilder normal_builder(Texture::Type::U8);

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

  auto handle_faker = [](Texture &&tex) -> Handle<Texture> {
    return Handle<Texture>(
        std::make_shared<Handle<Texture>::shared::element_type>(std::make_optional(std::move(tex))),
        AssetId(-1));
  };

  this->diffuse_texture =
      diffuse_builder.good() ? std::make_optional(handle_faker(diffuse_builder.build())) : std::nullopt;
  this->specular_texture =
      specular_builder.good() ? std::make_optional(handle_faker(specular_builder.build())) : std::nullopt;
  this->metallic_texture =
      metallic_builder.good() ? std::make_optional(handle_faker(metallic_builder.build())) : std::nullopt;
  this->specular_texture = this->specular_texture ? this->specular_texture : this->metallic_texture;

  this->emission_texture =
      emit_builder.good() ? std::make_optional(handle_faker(emit_builder.build())) : std::nullopt;
  this->normal_texture =
      normal_builder.good() ? std::make_optional(handle_faker(normal_builder.build())) : std::nullopt;

  // if (def.metallic.b != "") {
  //   this->shader = mngr.get<Shader>("gbuffer_pbr");
  // } else {
  //   this->shader = mngr.get<Shader>("gbuffer_generic");
  // }

  std::cout << "genereated material:" << std::endl;
  std::cout << "diffuse:" << this->diffuse_texture.has_value() << std::endl;
  std::cout << "specular:" << this->specular_texture.has_value() << std::endl;
  std::cout << "emission:" << this->emission_texture.has_value() << std::endl;
  std::cout << "normal:" << this->normal_texture.has_value() << std::endl;
}

PbrMaterial PbrMaterial::gold() {
  return PbrMaterial({0.003, 0.00225, 0.001}, {0.6, 0.4, 0.1}, 10);
}

PbrMaterial PbrMaterial::from_tinyobj(const tinyobj::material_t &material, const std::string &path) {
  PbrMaterial new_material;

  auto path_if = [](const std::string& path, const std::string &file) {
    if (file != "") {
      return path + file;
    }
    return std::string("");
  };

  PbrMaterial::definition spec {
      .diffuse = {glm::vec4(material.diffuse[0], material.diffuse[1], material.diffuse[2], 0.f),
                  path_if(path, material.diffuse_texname)},
      .specular = {glm::vec4(material.specular[0], material.specular[1], material.specular[2], 0.f),
                   path_if(path, material.specular_texname)},

      .emit = {glm::vec4(material.emission[0], material.emission[1], material.emission[2], 0.f),
               path_if(path, material.emissive_texname)},
      .roughness = {material.roughness, path_if(path, material.roughness_texname)},
      .alpha = {1 - material.dissolve, path_if(path, material.alpha_texname)},
      .metallic = {material.metallic, path_if(path, material.metallic_texname)},
  };

  return PbrMaterial(spec);
}
