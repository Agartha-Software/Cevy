/*
** Agartha-Software, 2024
** C++evy
** File description:
** Deferred renderer
*/

#include <optional>
#define GLM_FORCE_SWIZZLE

#if (_WIN32)
#include <GL/gl3w.h>
#endif
#if (__linux__)
#include <GL/glew.h>
#endif
#include <glm/gtc/type_ptr.hpp>

#include "DeferredRenderer.hpp"
#include "World.hpp"

static void renderQuad() {
  static uint quadVAO = 0;
  static uint quadVBO;
  if (quadVAO == 0) {
    std::cout << "initting quad" << std::endl;
    float quadVertices[] = {
        // positions        // texture Coords
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
    };
    // setup plane VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
  }
  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}

static glm::vec3 filmicToneMapping(glm::vec3 color) {
  color = max(glm::vec3(0.), color - glm::vec3(0.004));
  color = (color * (6.2f * color + .5f)) / (color * (6.2f * color + 1.7f) + 0.06f);
  return color;
}

void cevy::engine::DeferredRenderer::init() {
  this->alive = "DeferredRenderer is initialized";
  // std::cerr << " <<<< DeferredRenderer::init() <<<<" << std::endl;

  this->defaultMaterial = PbrMaterial();

  std::cout << "loading principled_shader" << std::endl;

  this->principled_shader = std::make_unique<ShaderProgram>();

  std::cout << "allocated principled_shader" << std::endl;

  this->principled_shader->initFromFiles("shaders/deferred.vert", "shaders/deferred.frag");
  std::cout << "inited principled_shader" << std::endl;

  this->principled_shader->addUniform("view");
  this->principled_shader->addUniform("invView");
  this->principled_shader->addUniform("ambientColor");
  this->principled_shader->addUniform("fog");
  this->principled_shader->addUniform("fog_far");
  this->principled_shader->addUniform("activeLights");

  GLuint uniformBlockIndexLights =
      glGetUniformBlockIndex(this->principled_shader->id(), "LightBlock");
  std::cout << "uniformBlockIndexLights: " << uniformBlockIndexLights << std::endl;
  glUniformBlockBinding(this->principled_shader->id(), uniformBlockIndexLights, 0);

  glGenBuffers(1, &this->uboLights);
  glBindBuffer(GL_UNIFORM_BUFFER, this->uboLights);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(pipeline::Light) * pipeline::Light::count, NULL,
               GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, uniformBlockIndexLights, this->uboLights);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  glBindBufferBase(GL_UNIFORM_BUFFER, 1, this->uboLights);

  std::cout << "loading gBuffer_shader" << std::endl;

  this->gBuffer_shader = std::make_unique<ShaderProgram>();

  std::cout << "allocated gBuffer_shader" << std::endl;

  this->gBuffer_shader->initFromFiles("shaders/simple.vert", "shaders/gbuffer.frag");
  std::cout << "inited gBuffer_shader" << std::endl;

  this->gBuffer_shader->addUniform("model");
  this->gBuffer_shader->addUniform("model_normal");
  this->gBuffer_shader->addUniform("view");
  this->gBuffer_shader->addUniform("invView");
  this->gBuffer_shader->addUniform("emit");
  this->gBuffer_shader->addUniform("emit_ambient");
  // this->gBuffer_shader->addUniform("fog");
  // this->gBuffer_shader->addUniform("fog_far");
  this->gBuffer_shader->addUniform("albedo");
  this->gBuffer_shader->addUniform("specular_tint");
  this->gBuffer_shader->addUniform("phong_exponent");
  this->gBuffer_shader->addUniform("halflambert");
  // this->gBuffer_shader->addUniform("activeLights");

  this->gbuffer.init_default();
}

void cevy::engine::DeferredRenderer::render(
    Query<Camera> cams,
    Query<option<Transform>, Handle<Model>, option<Handle<PbrMaterial>>, option<Color>> models,
    Query<option<Transform>, cevy::engine::PointLight> lights, const ecs::World &world) {

  auto r_atmo = world.get_resource<const Atmosphere>();
  const auto &atmosphere = r_atmo.has_value() ? r_atmo->get() : cevy::engine::Atmosphere();

  auto fog = atmosphere.fog.as_vec().rgb();
  auto ambient = atmosphere.ambiant.as_vec().rgb();
  auto fog_dist = atmosphere.fog_distance;

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  if (cams.size() == 0) {
    return;
  }
  auto &camera = std::get<Camera &>(*cams.get_single());

  this->gbuffer.write();
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glm::vec4 far_pos = {0, 0, 0, camera.far};
  glClearBufferfv(GL_COLOR, 0, glm::value_ptr(far_pos));

  this->gBuffer_shader->use();

  auto view = glm::scale(camera.projection, glm::vec3(1, camera.aspect, 1)) * camera.view;

  auto invView = glm::inverse(camera.view);
  invView = invView / invView[3][3];

  glUniformMatrix4fv(this->gBuffer_shader->uniform("view"), 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(this->gBuffer_shader->uniform("invView"), 1, GL_FALSE,
                     glm::value_ptr(invView));

  // std::cout << "rendering " << models.size() << " models" << std::endl;
  for (auto [o_tm, h_model, o_h_material, o_color] : models) {
    auto tm = o_tm ? o_tm->get_world().mat4() : glm::mat4(1);
    auto model = h_model.get();
    glm::vec4 white = glm::vec4(1, 1, 1, 1);
    auto &color = o_color ? o_color.value().as_vec() : white;
    PbrMaterial &material = o_h_material ? *o_h_material->get() : this->defaultMaterial;

    glUniform3fv(this->gBuffer_shader->uniform("emit"), 1, glm::value_ptr(material.ambiant));
    glUniform1i(this->gBuffer_shader->uniform("emit_ambient"), true);
    glUniform3fv(this->gBuffer_shader->uniform("albedo"), 1,
                 glm::value_ptr(material.diffuse * color.xyz()));
    glUniform3fv(this->gBuffer_shader->uniform("specular_tint"), 1,
                 glm::value_ptr(material.specular_tint));
    glUniform1i(this->gBuffer_shader->uniform("halflambert"), true);
    glUniform1f(this->gBuffer_shader->uniform("phong_exponent"), material.phong_exponent);
    glUniformMatrix4fv(this->gBuffer_shader->uniform("model"), 1, GL_FALSE,
                       glm::value_ptr(tm * model->modelMatrix()));
    glUniformMatrix3fv(this->gBuffer_shader->uniform("model_normal"), 1, GL_TRUE,
                       glm::value_ptr(model->tNormalMatrix() * glm::inverse(glm::mat3(tm))));
    // glUniform1i(this->gBuffer_shader->uniform("has_texture"), model->tex_coordinates.size() !=
    // 0);

    // if (material.diffuse_texture.has_value()) {
    //   glActiveTexture(GL_TEXTURE0);
    //   glBindTexture(GL_TEXTURE_2D, material.diffuse_texture->gl_handle);
    // }
    model->draw();
  };

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glClearColor(fog.r, fog.g, fog.b, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  this->principled_shader->use();
  this->gbuffer.read();

  std::vector<pipeline::Light> light_buffer;
  light_buffer.clear();
  light_buffer.reserve(pipeline::Light::count);

  for (auto [o_tm, light] : lights) {
    if (light_buffer.size() >= pipeline::Light::count)
      break;
    light_buffer.push_back(
        pipeline::Light(light, o_tm.has_value() ? o_tm->get_world().position : glm::vec3()));
  }

  glBindBuffer(GL_UNIFORM_BUFFER, this->uboLights);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, light_buffer.size() * sizeof(pipeline::Light),
                  light_buffer.data());
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  glUniform1i(this->principled_shader->uniform("activeLights"), light_buffer.size());
  // std::cout << "activeLights: " << light_buffer.size() << std::endl;

  glUniform3fv(this->principled_shader->uniform("ambientColor"), 1, glm::value_ptr(ambient));
  glUniform3fv(this->principled_shader->uniform("fog"), 1, glm::value_ptr(fog));

  glUniform1f(this->principled_shader->uniform("fog_far"), std::min(camera.far, fog_dist));

  glUniformMatrix4fv(this->principled_shader->uniform("view"), 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(this->principled_shader->uniform("invView"), 1, GL_FALSE,
                     glm::value_ptr(invView));

  renderQuad();
}
