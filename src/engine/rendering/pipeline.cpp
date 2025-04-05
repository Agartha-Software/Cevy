/*
** Agartha-Software, 2023
** C++evy
** File description:
** default pipeline builders
*/

#include "pipeline.hpp"

using cevy::engine::pipeline;
using cevy::engine::ShaderBuilder;

template <>
void ShaderBuilder<pipeline>::build(ShaderProgram &shader) {
  shader.addUniform(pipeline::uniforms::model::name);
  shader.addUniform(pipeline::uniforms::model_normal::name);
  shader.addUniform(pipeline::uniforms::view::name);
  shader.addUniform(pipeline::uniforms::invView::name);

  shader.addUniform(pipeline::uniforms::pbrMaterial::custom_ambient::name);
  shader.addUniform(pipeline::uniforms::pbrMaterial::diffuse::name);
  shader.addUniform(pipeline::uniforms::pbrMaterial::specular::name);
  shader.addUniform(pipeline::uniforms::pbrMaterial::roughness::name);
  shader.addUniform(pipeline::uniforms::pbrMaterial::emit::name);
  shader.addUniform(pipeline::uniforms::pbrMaterial::halflambert::name);
  shader.addUniform(pipeline::uniforms::pbrMaterial::normal_mode::name);
}
