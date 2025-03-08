/*
** Agartha-Software, 2023
** C++evy
** File description:
** default pipeline builders
*/

#include "pipeline.hpp"

using cevy::engine::pipeline;
using cevy::engine::ShaderBuilder;



template<>
void ShaderBuilder<pipeline>::build(ShaderProgram &shader) {
  shader.addUniform(pipeline::uniforms::model::name);
  shader.addUniform(pipeline::uniforms::model_normal::name);
  shader.addUniform(pipeline::uniforms::view::name);
  shader.addUniform(pipeline::uniforms::invView::name);

  shader.addUniform(pipeline::uniforms::PbrMaterial::custom_ambient::name);
  shader.addUniform(pipeline::uniforms::PbrMaterial::diffuse::name);
  shader.addUniform(pipeline::uniforms::PbrMaterial::specular::name);
  shader.addUniform(pipeline::uniforms::PbrMaterial::roughness::name);
  shader.addUniform(pipeline::uniforms::PbrMaterial::emit::name);
  shader.addUniform(pipeline::uniforms::PbrMaterial::halflambert::name);
  shader.addUniform(pipeline::uniforms::PbrMaterial::normal_mode::name);
}
