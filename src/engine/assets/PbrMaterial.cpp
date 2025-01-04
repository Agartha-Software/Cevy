#if (_WIN32)
#include <GL/gl3w.h>
#endif
#if (__linux__)
#include <GL/glew.h>
#endif

#include "GL/gl.h"
#include "PbrMaterial.hpp"
#include "stb_image.h"
#include <iostream>
#include <optional>

std::optional<cevy::engine::Texture>
cevy::engine::Texture::from_tinyobj(const std::string &file_name,
                                    const tinyobj::texture_option_t & /* _option */) {
  Texture new_texture;
  new_texture.file_name = file_name;

  if (file_name != "") {
    int width, height, nrChannels;
    std::string path = "./assets/" + file_name;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *image_data = stbi_load(&path[0], &width, &height, &nrChannels, 0);

    if (!image_data) {
      std::cerr << "Error: Failed to load [" << path << "] texture" << std::endl;
      stbi_image_free(image_data);
      return std::optional<Texture>();
    }

    glGenTextures(1, &new_texture.gl_handle);
    glBindTexture(GL_TEXTURE_2D, new_texture.gl_handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 image_data); // HERE
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(image_data);

    return new_texture;
  }
  return std::optional<Texture>();
}

cevy::engine::PbrMaterial cevy::engine::PbrMaterial::gold() {
  return PbrMaterial({0.003, 0.00225, 0.001}, {0.6, 0.4, 0.1}, 10);
}

cevy::engine::PbrMaterial
cevy::engine::PbrMaterial::from_tinyobj(const tinyobj::material_t &material) {
  PbrMaterial new_material;

  printf("name %s\n", material.name.c_str());
  new_material.ambiant = {material.ambient[0], material.ambient[1], material.ambient[2]};
  printf("ambiant %f %f %f \n", new_material.ambiant.x, new_material.ambiant.y,
         new_material.ambiant.z);
  new_material.diffuse = {material.diffuse[0], material.diffuse[1], material.diffuse[2]};
  printf("diffuse %f %f %f \n", new_material.diffuse.x, new_material.diffuse.y,
         new_material.diffuse.z);
  new_material.specular_tint = {material.specular[0], material.specular[1], material.specular[2]};
  printf("spec %f %f %f \n", new_material.specular_tint.x, new_material.specular_tint.y,
         new_material.specular_tint.z);
  new_material.diffuse_texture =
      Texture::from_tinyobj(material.diffuse_texname, material.diffuse_texopt);
  printf("name '%s'\n", new_material.diffuse_texture->file_name.c_str());
  return new_material;
}
