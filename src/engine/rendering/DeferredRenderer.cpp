/*
** Agartha-Software, 2024
** C++evy
** File description:
** Deferred renderer
*/

#define GLM_FORCE_SWIZZLE

#include "glx.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "Atmosphere.hpp"
#include "DeferredRenderer.hpp"
#include "Model.hpp"
#include "World.hpp"

// static void renderQuad() {
//   static uint quadVAO = 0;
//   static uint quadVBO;
//   if (quadVAO == 0) {
//     std::cout << "initting quad" << std::endl;
//     float quadVertices[] = {
//         // positions        // texture Coords
//         -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
//         1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
//     };
//     // setup plane VAO
//     glGenVertexArrays(1, &quadVAO);
//     glGenBuffers(1, &quadVBO);
//     glBindVertexArray(quadVAO);
//     glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
//     glEnableVertexAttribArray(1);
//     glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 *
//     sizeof(float)));
//   }
//   glBindVertexArray(quadVAO);
//   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//   glBindVertexArray(0);
// }

// static glm::vec3 filmicToneMapping(glm::vec3 color) {
//   color = max(glm::vec3(0.), color - glm::vec3(0.004));
//   color = (color * (6.2f * color + .5f)) / (color * (6.2f * color + 1.7f) + 0.06f);
//   return color;
// }

void cevy::engine::DeferredRenderer::init() {
  this->alive = "DeferredRenderer is initialized";
  // std::cerr << " <<<< DeferredRenderer::init() <<<<" << std::endl;

  this->defaultMaterial = PbrMaterial();

  this->null_shader = std::make_unique<ShaderProgram>();
  this->null_shader->initFromFiles("assets/engine/shaders/simple.vert",
                                   "assets/engine/shaders/empty.frag");
  this->null_shader->addUniform("model");
  this->null_shader->addUniform("view");

  std::cout << "loading compose_shader" << std::endl;

  this->compose_shader = std::make_unique<ShaderProgram>();

  std::cout << "allocated compose_shader" << std::endl;

  this->compose_shader->initFromFiles("assets/engine/shaders/deferred.vert",
                                      "assets/engine/shaders/deferred_compose.frag");
  std::cout << "inited compose_shader" << std::endl;

  this->compose_shader->addUniform("width");
  this->compose_shader->addUniform("height");
  this->compose_shader->addUniform("canvas");
  this->compose_shader->addUniform("view");
  this->compose_shader->addUniform("invView");
  this->compose_shader->addUniform("ambientColor");
  this->compose_shader->addUniform("fog");
  this->compose_shader->addUniform("fog_far");
  this->compose_shader->addUniform("exposure");

  std::cout << "loading accumulate_shader" << std::endl;

  this->accumulate_shader = std::make_unique<ShaderProgram>();

  std::cout << "allocated accumulate_shader" << std::endl;

  this->accumulate_shader->initFromFiles("assets/engine/shaders/deferred.vert",
                                         "assets/engine/shaders/deferred_accumulate.frag");
  std::cout << "inited accumulate_shader" << std::endl;

  this->accumulate_shader->addUniform("width");
  this->accumulate_shader->addUniform("height");
  this->accumulate_shader->addUniform("canvas");
  this->accumulate_shader->addUniform("view");
  this->accumulate_shader->addUniform("invView");
  this->accumulate_shader->addUniform("debug_draw");
  this->accumulate_shader->addUniform("lightPosition");
  this->accumulate_shader->addUniform("lightEnergy");
  this->accumulate_shader->addUniform("lightRadius");
  this->accumulate_shader->addUniform("lightRange");

  std::cout << "loading gBuffer_shader" << std::endl;

  this->gBuffer_shader = std::make_unique<ShaderProgram>();

  std::cout << "allocated gBuffer_shader" << std::endl;

  this->gBuffer_shader->initFromFiles("assets/engine/shaders/simple.vert",
                                      "assets/engine/shaders/gbuffer.frag");
  std::cout << "inited gBuffer_shader" << std::endl;

  this->gBuffer_shader->addUniform("model");
  this->gBuffer_shader->addUniform("model_normal");
  this->gBuffer_shader->addUniform("view");
  this->gBuffer_shader->addUniform("invView");
  this->gBuffer_shader->addUniform("custom_ambient");
  this->gBuffer_shader->addUniform("diffuse_const");
  this->gBuffer_shader->addUniform("specular_const");
  this->gBuffer_shader->addUniform("roughness_const");
  this->gBuffer_shader->addUniform("emit_const");
  this->gBuffer_shader->addUniform("halflambert");

  this->gbuffer.init_default();
  this->billboard.init();

  this->primitives.sphere = primitives::sphere(1, 10, 6);
  this->primitives.blank = TextureBuilder::from(glm::vec4u8(1, 1, 1, 1), 2, 2);
  // this->primitives.black = TextureBuilder::from(glm::vec4u8(0, 0, 0, 1), 2, 2);
  // this->primitives.flat = TextureBuilder::from(glm::vec4(0.5, 0.5, 0.5, 1), 2, 2);
}

void cevy::engine::DeferredRenderer::render_system(
    DeferredRenderer &self, Query<Camera> cams,
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

  self.gbuffer.write();
  glDepthMask(GL_TRUE);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glm::vec4 far_pos = {0, 0, 0, camera.far};
  glClearBufferfv(GL_COLOR, 1, glm::value_ptr(far_pos));

  self.gBuffer_shader->use();

  auto invView = camera.view;
  auto view =
      glm::scale(camera.projection, glm::vec3(1, camera.aspect, 1)) * glm::inverse(camera.view);
  view = view / std::abs(view[3][3]);

  // invView = invView / invView[3][3];

  glUniformMatrix4fv(self.gBuffer_shader->uniform("view"), 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(self.gBuffer_shader->uniform("invView"), 1, GL_FALSE, glm::value_ptr(invView));

  for (auto [o_tm, model, o_material, o_color] : models) {
    auto tm = o_tm ? o_tm->get_world().mat4() : glm::mat4(1);
    glm::vec4 white = glm::vec4(1, 1, 1, 1);
    auto &color = o_color ? o_color.value().as_vec() : white;
    PbrMaterial &material = o_material ? o_material->get() : self.defaultMaterial;

    glUniform3fv(self.gBuffer_shader->uniform("custom_ambient"), 1,
                 glm::value_ptr(material.ambient));
    glUniform3fv(self.gBuffer_shader->uniform("emit_const"), 1, glm::value_ptr(material.emit));
    glUniform3fv(self.gBuffer_shader->uniform("diffuse_const"), 1,
                 glm::value_ptr(material.diffuse * color.xyz()));
    glUniform3fv(self.gBuffer_shader->uniform("specular_const"), 1,
                 glm::value_ptr(material.specular_tint));
    glUniform1f(self.gBuffer_shader->uniform("roughness_const"), 1 / material.phong_exponent);
    glUniform1i(self.gBuffer_shader->uniform("halflambert"), material.halflambert);
    glUniformMatrix4fv(self.gBuffer_shader->uniform("model"), 1, GL_FALSE,
                       glm::value_ptr(tm * model->modelMatrix()));
    glUniformMatrix3fv(self.gBuffer_shader->uniform("model_normal"), 1, GL_TRUE,
                       glm::value_ptr(model->tNormalMatrix() * glm::inverse(glm::mat3(tm))));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material.diffuse_texture.has_value()
                                     ? material.diffuse_texture.value()->texture_handle()
                                     : self.primitives.blank.texture_handle());

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, material.specular_texture.has_value()
                                     ? material.specular_texture.value()->texture_handle()
                                     : self.primitives.blank.texture_handle());

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, material.emission_texture.has_value()
                                     ? material.emission_texture.value()->texture_handle()
                                     : self.primitives.blank.texture_handle());
    model->draw();
  };

  self.gbuffer.read();
  glDepthMask(GL_FALSE);

  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_ONE, GL_ONE);

  glClearStencil(0);

  for (auto [o_tm, light] : lights) {
    const auto &pos = o_tm.has_value() ? o_tm->get_world().position : glm::vec3();
    pipeline::Light gl_light(light, pos);

#if 0  // use stencil
    glEnable(GL_STENCIL_TEST);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilMask(0xff);

    glColorMask(false, false, false, false);
    glStencilFunc(GL_ALWAYS, 0xff, 0xff);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    self.null_shader->use();
    glUniformMatrix4fv(self.null_shader->uniform("view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(self.null_shader->uniform("model"), 1, GL_FALSE,
                       glm::value_ptr(gl_light.model));
    self.primitives.sphere.draw();

    glStencilFunc(GL_EQUAL, 0x1, 0xff);

    glColorMask(true, true, true, true);
#endif // use stencil

    self.accumulate_shader->use();
    glUniform3fv(self.accumulate_shader->uniform("lightPosition"), 1, glm::value_ptr(pos));
    glUniform3fv(self.accumulate_shader->uniform("lightEnergy"), 1, glm::value_ptr(gl_light.color));
    glUniform1f(self.accumulate_shader->uniform("lightRadius"), gl_light.radius);
    glUniform1i(self.accumulate_shader->uniform("debug_draw"), 0);

    glUniform1f(self.accumulate_shader->uniform("width"), self.width);
    glUniform1f(self.accumulate_shader->uniform("height"), self.height);

    glUniformMatrix4fv(self.accumulate_shader->uniform("invView"), 1, GL_FALSE,
                       glm::value_ptr(invView));
    glUniformMatrix4fv(self.accumulate_shader->uniform("view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniform3fv(self.accumulate_shader->uniform("lightEnergy"), 1, glm::value_ptr(gl_light.color));
    glUniformMatrix4fv(self.accumulate_shader->uniform("canvas"), 1, GL_FALSE,
                       glm::value_ptr(view * gl_light.model));
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    self.primitives.sphere.draw();
    // glDisable(GL_CULL_FACE);
    // self.billboard.draw();
  }

  glDisable(GL_STENCIL_TEST);

  glDisable(GL_BLEND);
  glCullFace(GL_BACK);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  self.compose_shader->use();

  glUniform1f(self.compose_shader->uniform("width"), self.width);
  glUniform1f(self.compose_shader->uniform("height"), self.height);
  glUniformMatrix4fv(self.compose_shader->uniform("canvas"), 1, GL_FALSE,
                     glm::value_ptr(glm::mat4(1)));
  glUniformMatrix4fv(self.compose_shader->uniform("view"), 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(self.compose_shader->uniform("invView"), 1, GL_FALSE, glm::value_ptr(invView));
  glUniform3fv(self.compose_shader->uniform("ambientColor"), 1, glm::value_ptr(ambient));
  glUniform3fv(self.compose_shader->uniform("fog"), 1, glm::value_ptr(fog));
  glUniform1f(self.compose_shader->uniform("fog_far"), std::min(camera.far, fog_dist));
  self.billboard.screenspace({-1, -1}, {1, 1});
  self.billboard.draw();
}
