/*
** Agartha-Software, 2024
** C++evy
** File description:
** Deferred renderer
*/

#define GLM_FORCE_SWIZZLE

#include "glx.hpp"
#include <cmath>
#include <memory>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>
#include <stdexcept>
#include <cassert>

#include "Atmosphere.hpp"
#include "DeferredRenderer.hpp"
#include "Mesh.hpp"
#include "World.hpp"
#include "Time.hpp"

template <>
void cevy::engine::ShaderBuilder<cevy::engine::DeferredRenderer::pipeline>::build(
    ShaderProgram &shader) {
  cevy::engine::ShaderBuilder<cevy::engine::pipeline>::build(shader);
};

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


inline glm::mat4 operator*(const glm::mat4 &m, const glm::vec3 &v2) {
  return m * glm::mat4(v2.x, 0, 0, 0, //
                       0, v2.y, 0, 0, //
                       0, 0, v2.z, 0, //
                       0, 0, 0, 1);
};

void cevy::engine::DeferredRenderer::init(glWindow &) {
  this->alive = "DeferredRenderer is initialized";
  // std::cerr << " <<<< DeferredRenderer::init() <<<<" << std::endl;

  // this->defaultMaterial = Handle(PbrMaterial());

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
  this->accumulate_shader->addUniform("projector");
  this->accumulate_shader->addUniform("view");
  this->accumulate_shader->addUniform("invView");
  this->accumulate_shader->addUniform("debug_draw");
  this->accumulate_shader->addUniform("lightPosition");
  this->accumulate_shader->addUniform("lightDirection");
  this->accumulate_shader->addUniform("lightEnergy");
  this->accumulate_shader->addUniform("lightAngle");
  this->accumulate_shader->addUniform("lightRadius");
  this->accumulate_shader->addUniform("lightRange");
  this->accumulate_shader->addUniform("lightFade");
  this->accumulate_shader->addUniform("lightType");

  std::cout << "loading gBuffer_shader" << std::endl;

  // this->defaultMaterial->shader.emplace(Handle<ShaderProgram>(ShaderProgram()));
  // auto &gBuffer_shader = this->defaultMaterial->shader.value();
  auto &gBuffer_shader = this->defaultShader;
  gBuffer_shader = std::make_unique<ShaderProgram>();

  std::cout << "allocated gBuffer_shader" << std::endl;

  gBuffer_shader->initFromFiles("assets/engine/shaders/simple.vert",
                                "assets/engine/shaders/gbuffer_generic.frag");
  std::cout << "inited gBuffer_shader" << std::endl;

  gBuffer_shader->addUniform("model");
  gBuffer_shader->addUniform("model_normal");
  gBuffer_shader->addUniform("view");
  gBuffer_shader->addUniform("invView");
  gBuffer_shader->addUniform("custom_ambient");
  gBuffer_shader->addUniform("diffuse_const");
  gBuffer_shader->addUniform("specular_const");
  gBuffer_shader->addUniform("roughness_const");
  gBuffer_shader->addUniform("emit_const");
  gBuffer_shader->addUniform("halflambert");
  gBuffer_shader->addUniform("normal_mode");

  this->gbuffer.init_default();
  this->billboard.init();

  this->shadowMap.init();

  {
    int s_stacks = 4;
    int s_slices = 6;
    float s_st_len = glm::pi<float>() * 2 / (s_stacks * 2);
    float s_sl_len = glm::pi<float>() * 2 / s_slices;
    float s_diag_len_2 = s_st_len * s_st_len + s_sl_len * s_sl_len;
    float s_diag_error = std::sqrt(1 * 1 - (s_diag_len_2 / 4));
    this->primitives.sphere = primitives::sphere(1 / s_diag_error, s_slices, s_stacks);
  }

  this->primitives.cube = primitives::cube(1);
  this->primitives.blank = TextureBuilder::from(glm::vec4u8(255, 255, 255, 127), 2, 2);
  // this->primitives.black = TextureBuilder::from(glm::vec4u8(0, 0, 0, 1), 2, 2);
  this->primitives.flat = TextureBuilder::from(glm::vec4(0.5, 0.5, 1, 1), 2, 2);
}

void cevy::engine::DeferredRenderer::deinit(glWindow &) {}

void cevy::engine::DeferredRenderer::render_system(
    Resource<Window> win, Query<Camera> cams,
    Query<option<Transform>, Handle<Mesh>, option<Handle<PbrMaterial>>, option<Color>> models,
    Query<option<Transform>, option<cevy::engine::PointLight>, option<cevy::engine::SpotLight>,  option<cevy::engine::SunLight>>
        lights,
    const ecs::World &world) {
  auto &window = win->get_handler<glWindow>();
  auto target_size = window.getTargetSize();

  DeferredRenderer &self = window.get_module<DeferredRenderer>();

  // static float last_time = 0;
  // auto time = world.resource<cevy::ecs::Time>();
  // last_time = 0.9 * last_time + 0.1 * time.delta_seconds();
  // last_time = 0.9 * last_time + 0.1 * time.delta_seconds();
  // std::cout << std::fixed << "frame_time: " << time.delta_seconds() << " (" << 1 / time.delta_seconds() << "), (" << 1 / last_time << ")" << std::endl;

  const auto &r_atmo = world.get_resource<const Atmosphere>();
  const auto &atmosphere = r_atmo.has_value() ? r_atmo->get() : cevy::engine::Atmosphere();

  auto fog = atmosphere.fog.as_vec().rgb();
  auto ambient = atmosphere.ambiant.as_vec().rgb();
  auto fog_dist = atmosphere.fog_distance;

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glViewport(0, 0, self.width, self.height);

  if (cams.size() == 0) {
    return;
  }
  auto &camera = std::get<Camera &>(cams.single());

  self.gbuffer.write();
  glDepthMask(GL_TRUE);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glm::vec4 far_pos = {0, 0, 0, camera.far};
  glClearBufferfv(GL_COLOR, 1, glm::value_ptr(far_pos));

  // self.gBuffer_shader->use();

  auto &view = self.renderContext.view;
  view = glm::scale(camera.projection, glm::vec3(1, camera.aspect, 1)) * camera.view;

  auto &invView = self.renderContext.invView;
  invView = glm::inverse(camera.view);
  // invView = invView / invView[3][3];

  self.renderContext.models.clear();

  for (const auto &[o_tm, model, o_material, o_color] : models) {
    auto tm = o_tm ? o_tm->get_world().mat4() : glm::mat4(1);
    glm::vec4 white = glm::vec4(1, 1, 1, 1);
    const auto &color = o_color ? o_color.value().as_vec() : white;
    // TODO: reorganize for support default material;
    if (!o_material)
      continue;
    const Handle<PbrMaterial>& material = o_material.value();
    self.renderContext.models[std::make_pair(model, material)].push_back({tm, color});
  }

  const Shader *active_shader = nullptr; //self.defaultShader.get();

  for (const auto &[mesh_mat, instances] : self.renderContext.models) {
    const auto &[mesh, material] = mesh_mat;

    const auto &shader =
        material->shader ? material->shader->get() : *self.defaultShader;
        // material->shader ? material->shader->get() : self.defaultMaterial->shader->get();

    if (&shader != active_shader) {
      shader.use();
      glUniformMatrix4fv(shader.uniform("view"), 1, GL_FALSE, glm::value_ptr(view));
      glUniformMatrix4fv(shader.uniform("invView"), 1, GL_FALSE, glm::value_ptr(invView));
      active_shader = &shader;
    }

    glUniform3fv(shader.uniform("custom_ambient"), 1, glm::value_ptr(material->ambient));
    glUniform3fv(shader.uniform("emit_const"), 1, glm::value_ptr(material->emit));
    glUniform3fv(shader.uniform("specular_const"), 1, glm::value_ptr(material->specular_tint));
    glUniform1f(shader.uniform("roughness_const"), material->roughness);
    glUniform1i(shader.uniform("normal_mode"),
                int(pipeline::uniforms::NormalMode::Tangeant) * mesh->hasTangeants());
    glUniform1i(shader.uniform("halflambert"), material->halflambert);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material->diffuse_texture.has_value()
                                      ? material->diffuse_texture.value()->texture_handle()
                                      : self.primitives.blank.texture_handle());

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, material->specular_texture.has_value()
                                      ? material->specular_texture.value()->texture_handle()
                                      : self.primitives.blank.texture_handle());

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, material->emission_texture.has_value()
                                      ? material->emission_texture.value()->texture_handle()
                                      : self.primitives.black.texture_handle());
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, material->normal_texture.has_value()
                                      ? material->normal_texture.value()->texture_handle()
                                      : self.primitives.flat.texture_handle());

    for (const auto &instance : instances) {
      glUniform3fv(shader.uniform("diffuse_const"), 1,
                   glm::value_ptr(material->diffuse * instance.color.xyz()));
      glUniformMatrix4fv(shader.uniform("model"), 1, GL_FALSE,
                         glm::value_ptr(instance.matrix * mesh->modelMatrix()));
      glUniformMatrix3fv(
          shader.uniform("model_normal"), 1, GL_TRUE,
          glm::value_ptr(mesh->tNormalMatrix() * glm::inverse(glm::mat3(instance.matrix))));
      mesh->draw();
    }
  };

  for (const auto &[o_tm, o_point, o_spot, o_sun] : lights) {
    const auto &tm = o_tm.has_value() ? o_tm->get_world() : Transform();

    if (all(!o_point.has_value(), !o_spot.has_value(), !o_sun.has_value()))
      continue;

    pipeline::Light gl_light = o_point.has_value() ? pipeline::Light(o_point.value(), tm) :       //
                                   (o_spot.has_value() ? pipeline::Light(o_spot.value(), tm) :    //
                                        (o_sun.has_value() ? pipeline::Light(o_sun.value(), tm) : //
                                             throw std::runtime_error("")));
    self.light_pass(gl_light);
  }

  self.renderContext.models.clear();

  self.gbuffer.read();

  glDisable(GL_STENCIL_TEST);

  glDisable(GL_BLEND);
  glCullFace(GL_BACK);

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


  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, window.getCurrentFrameBuffer());

  glBindFramebuffer(GL_READ_FRAMEBUFFER, self.gbuffer.getFramebuffer());
  // glBindFramebuffer(GL_READ_FRAMEBUFFER, window.getCurrentFrameBuffer());

  // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, window.getCurrentFrameBuffer());

  // auto factor = std::min(target_size.x / float(self.width), target_size.y / float(self.height));

  // auto left = (target_size.x - factor * self.width);
  // auto bottom = (target_size.y - factor * self.height);

  auto factor = std::max(target_size.x / float(self.width), target_size.y / float(self.height));
   auto left = (target_size.x - factor * self.width) / 2;
  auto bottom = (target_size.y - factor * self.height) / 2;

  // glBlitFramebuffer(0, 0, self.width, self.height, 0, 0, self.width, self.height,
  //                 GL_COLOR_BUFFER_BIT, GL_LINEAR);


  // glBlitFramebuffer(0, 0, self.width, self.height, 0, 0, window_size.x, window_size.y,
  //                 GL_COLOR_BUFFER_BIT, GL_LINEAR);
  // glBlitFramebuffer(0, 0, self.width, self.height, 0, 0, target_size.x, target_size.y,
  //                 GL_COLOR_BUFFER_BIT, GL_LINEAR);



  // glBlitFramebuffer(0, 0, window_size.x, window_size.y, 0, 0,self.width, self.height,
  //                 GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glBlitFramebuffer(0, 0, self.width, self.height, left, bottom, factor * self.width, factor * self.height,
                  GL_COLOR_BUFFER_BIT, GL_LINEAR);

  // glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // glBindFramebuffer(GL_TEXTURE_2D, 0);
}

void cevy::engine::DeferredRenderer::light_pass(const pipeline::Light &light) {
  auto light_direction = glm::normalize((light.model * glm::vec4(0, 0, -1, 0)).xyz());
  auto projector = glm::inverse(light.model);
  auto persp = glm::mat4(1);
  auto squash = glm::mat4(1);
  if (light.type == pipeline::Light::Type::Spot) {
    persp = glm::perspective(light.angle * 2, 1.f, 0.01f, light.range);
    squash = glm::inverse(-persp);
    squash = squash / squash[3][3];
    squash = squash * glm::vec3(-1);
    // squash[3][3] = -1;
  } else if (light.type == pipeline::Light::Type::Sun) {
    persp = glm::ortho(-light.radius, light.radius, -light.radius, light.radius, -light.range, light.range);
    squash = glm::inverse(-persp);
    squash = squash / squash[3][3];
    squash = squash * glm::vec3(-1);
  } else {
    squash = glm::mat4(1) * glm::vec3(light.range);
  }
  projector = persp * projector;

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

  glDepthMask(GL_TRUE);

  glDisable(GL_BLEND);
  glCullFace(GL_BACK);
  glEnable(GL_DEPTH_TEST);

  { // shadow map
    glViewport(0, 0, this->shadowMap.size().x, this->shadowMap.size().y);
    this->shadowMap.write();
    glClear(GL_DEPTH_BUFFER_BIT);
    this->null_shader->use();
    glColorMask(false, false, false, false);

    glUniformMatrix4fv(this->null_shader->uniform("view"), 1, GL_FALSE, glm::value_ptr(projector));

    for (const auto &[mesh_mat, instances] : this->renderContext.models) {
      const auto &[mesh, _mat] = mesh_mat;
      for (const auto &instance : instances) {
        glUniformMatrix4fv(this->null_shader->uniform("model"), 1, GL_FALSE,
                           glm::value_ptr(instance.matrix * mesh->modelMatrix()));
        mesh->draw();
      }
    }

    glColorMask(true, true, true, true);
  }

  this->gbuffer.read();

  glDepthMask(GL_FALSE);
  glEnable(GL_DEPTH_CLAMP);
  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_ONE, GL_ONE);

  glClearStencil(0);

  glDisable(GL_DEPTH_TEST);

  glCullFace(GL_FRONT);

  glViewport(0, 0, this->width, this->height);
  this->gbuffer.write();

  this->shadowMap.read(0);

  this->accumulate_shader->use();
  glUniform3fv(this->accumulate_shader->uniform("lightDirection"), 1,
               glm::value_ptr(light_direction));
  glUniform3fv(this->accumulate_shader->uniform("lightPosition"), 1,
               glm::value_ptr(light.model[3]));
  glUniform3fv(this->accumulate_shader->uniform("lightEnergy"), 1, glm::value_ptr(light.color));
  glUniform1f(this->accumulate_shader->uniform("lightRadius"), light.radius);
  glUniform1f(this->accumulate_shader->uniform("lightRange"), light.range);
  glUniform1f(this->accumulate_shader->uniform("lightFade"), 0.3);
  glUniform1f(this->accumulate_shader->uniform("lightAngle"), light.angle);
  glUniform1ui(this->accumulate_shader->uniform("lightType"), static_cast<uint32_t>(light.type));
  glUniform1i(this->accumulate_shader->uniform("debug_draw"), 0);

  glUniform1f(this->accumulate_shader->uniform("width"), this->width);
  glUniform1f(this->accumulate_shader->uniform("height"), this->height);

  glUniformMatrix4fv(this->accumulate_shader->uniform("invView"), 1, GL_FALSE,
                     glm::value_ptr(this->renderContext.invView));
  glUniformMatrix4fv(this->accumulate_shader->uniform("view"), 1, GL_FALSE,
                     glm::value_ptr(this->renderContext.view));
  glUniformMatrix4fv(this->accumulate_shader->uniform("canvas"), 1, GL_FALSE,
                     glm::value_ptr(this->renderContext.view * light.model * squash));
  glUniformMatrix4fv(this->accumulate_shader->uniform("projector"), 1, GL_FALSE,
                     glm::value_ptr(projector));

  if (light.type == pipeline::Light::Type::Point) {
    this->primitives.sphere.draw();
  } else {
    this->primitives.cube.draw();
  }
  glDisable(GL_DEPTH_CLAMP);
}
