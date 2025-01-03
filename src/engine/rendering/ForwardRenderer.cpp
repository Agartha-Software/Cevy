/*
** Agartha-Software, 2024
** C++evy
** File description:
** forward renderer
*/

#define GLM_FORCE_SWIZZLE

#if (_WIN32)
#include <GL/gl3w.h>
#endif
#if (__linux__)
#include <GL/glew.h>
#endif
#include <glm/gtc/type_ptr.hpp>

#include "ForwardRenderer.hpp"
#include "pipeline.hpp"

static glm::vec3 filmicToneMapping(glm::vec3 color) {
  color = max(glm::vec3(0.), color - glm::vec3(0.004));
  color = (color * (6.2f * color + .5f)) / (color * (6.2f * color + 1.7f) + 0.06f);
  return color;
}

void cevy::engine::ForwardRenderer::init() {
  this->defaultMaterial = PbrMaterial();

  env.ambientColor = {.1, .15, .2};
  env.ambientColor = env.ambientColor * env.ambientColor;
  env.fog = env.ambientColor;

  std::cout << "loading shaderProgram" << std::endl;

  this->shaderProgram = new ShaderProgram();

  std::cout << "allocated shaderProgram" << std::endl;

  this->shaderProgram->initFromFiles("shaders/simple.vert", "shaders/simple.frag");
  std::cout << "inited shaderProgram" << std::endl;

  this->shaderProgram->addUniform("model");
  this->shaderProgram->addUniform("model_normal");
  this->shaderProgram->addUniform("view");
  this->shaderProgram->addUniform("custom1");
  this->shaderProgram->addUniform("invView");
  this->shaderProgram->addUniform("ambientColor");
  this->shaderProgram->addUniform("fog");
  this->shaderProgram->addUniform("fog_far");
  this->shaderProgram->addUniform("albedo");
  this->shaderProgram->addUniform("specular_tint");
  this->shaderProgram->addUniform("phong_exponent");
  this->shaderProgram->addUniform("halflambert");
  this->shaderProgram->addUniform("activeLights");

  GLuint uniformBlockIndexLights = glGetUniformBlockIndex(this->shaderProgram->id(), "LightBlock");
  std::cout << "uniformBlockIndexLights: " << uniformBlockIndexLights << std::endl;
  glUniformBlockBinding(this->shaderProgram->id(), uniformBlockIndexLights, 0);

  glGenBuffers(1, &this->uboLights);
  glBindBuffer(GL_UNIFORM_BUFFER, this->uboLights);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(pipeline::Light) * pipeline::Light::count, NULL,
               GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, uniformBlockIndexLights, this->uboLights);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  glBindBufferBase(GL_UNIFORM_BUFFER, 1, this->uboLights);
}

void cevy::engine::ForwardRenderer::render(
    Query<Camera> cams,
    Query<option<Transform>, Handle<Model>, option<Handle<PbrMaterial>>, option<Color>> models,
    Query<option<Transform>, cevy::engine::PointLight> lights) {

  auto fog = filmicToneMapping(this->env.fog);
  glClearColor(fog.r, fog.g, fog.b, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);
  // glCullFace(GL_BACK);

  this->shaderProgram->use();

  if (cams.size() == 0) {
    return;
  }

  auto &camera = std::get<Camera &>(*cams.get_single());

  auto view = glm::scale(camera.projection, glm::vec3(1, camera.aspect, 1)) * camera.view;

  std::vector<pipeline::Light> light_buffer;
  light_buffer.clear();
  light_buffer.reserve(pipeline::Light::count);

  for (auto [o_tm, light] : lights) {
    if (light_buffer.size() >= pipeline::Light::count)
      break;
    // light_buffer.push_back(pipeline::Light(light, o_tm.has_value() ? o_tm->position :
    // glm::vec3()));
    light_buffer.push_back(
        pipeline::Light(light, o_tm.has_value() ? o_tm->get_world().position : glm::vec3()));
  }

  glBindBuffer(GL_UNIFORM_BUFFER, this->uboLights);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, light_buffer.size() * sizeof(pipeline::Light),
                  light_buffer.data());
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  glUniform1i(this->shaderProgram->uniform("activeLights"), light_buffer.size());
  // std::cout << "activeLights: " << light_buffer.size() << std::endl;

  glUniform3fv(this->shaderProgram->uniform("fog"), 1, glm::value_ptr(env.fog));

  glUniform1f(this->shaderProgram->uniform("fog_far"), camera.far);

  glUniformMatrix4fv(this->shaderProgram->uniform("view"), 1, GL_FALSE, glm::value_ptr(view));

  auto invView = glm::inverse(camera.view);
  invView = invView / invView[3][3];

  glUniformMatrix4fv(this->shaderProgram->uniform("invView"), 1, GL_FALSE, glm::value_ptr(invView));


  // std::cout << "rendering " << models.size() << " models" << std::endl;
  for (auto [o_tm, h_model, o_h_material, o_color] : models) {
    auto tm = o_tm ? o_tm->get_world().mat4() : glm::mat4(1);
    auto model = h_model.get();
    glm::vec4 white = glm::vec4(1, 1, 1, 1);
    auto &color = o_color ? o_color.value().as_vec() : white;
    PbrMaterial &material = o_h_material ? *o_h_material->get() : this->defaultMaterial;

    glUniform3fv(this->shaderProgram->uniform("ambientColor"), 1,
                 glm::value_ptr(env.ambientColor + material.ambiant));
    glUniform3fv(this->shaderProgram->uniform("albedo"), 1,
                 glm::value_ptr(material.diffuse * color.xyz()));
    glUniform3fv(this->shaderProgram->uniform("specular_tint"), 1,
                 glm::value_ptr(material.specular_tint));
    glUniform1f(this->shaderProgram->uniform("phong_exponent"), material.phong_exponent);
    glUniform1i(this->shaderProgram->uniform("halflambert"), true);
    glUniformMatrix4fv(this->shaderProgram->uniform("model"), 1, GL_FALSE,
                       glm::value_ptr(tm * model->modelMatrix()));
    glUniformMatrix3fv(this->shaderProgram->uniform("model_normal"), 1, GL_TRUE,
                       glm::value_ptr(model->tNormalMatrix() * glm::inverse(glm::mat3(tm))));
    glUniform1i(this->shaderProgram->uniform("has_texture"), model->tex_coordinates.size() != 0);

    if (material.diffuse_texture.has_value()) {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, material.diffuse_texture->gl_handle);
    }
    model->draw();
  };
}