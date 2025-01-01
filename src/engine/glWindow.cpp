/*
** Agartha-Software, 2024
** C++evy
** File description:
** openGL window handling
*/

#define GLM_FORCE_SWIZZLE

#include "World.hpp"
#include "Atmosphere.hpp"
#include "Scheduler.hpp"
#include <stdexcept>

#if (_WIN32)
#include <GL/gl3w.h>
#endif
#if (__linux__)
#include <GL/glew.h>
#endif
#include "glWindow.hpp"
#include <glm/gtc/type_ptr.hpp>

static glm::vec3 hsv2rgb(glm::vec3 c) {
  glm::vec4 K = glm::vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  glm::vec3 p = abs(fract(c.xxx() + K.xyz()) * 6.0f - K.www());
  return c.z * mix(K.xxx(), clamp(p - K.xxx(), 0.0f, 1.0f), c.y);
}

static glm::vec3 filmicToneMapping(glm::vec3 color) {
  color = max(glm::vec3(0.), color - glm::vec3(0.004));
  color = (color * (6.2f * color + .5f)) / (color * (6.2f * color + 1.7f) + 0.06f);
  return color;
}

glWindow::glWindow(int width, int height) : width(width), height(height) {
  open();
  this->defaultMaterial = PbrMaterial();
  this->setupEnv();
}

bool glWindow::open() {
  this->init_context();
  return 0;
}

glWindow::~glWindow() { close(); }

bool glWindow::close() {
  if (this->glfWindow) {
    glfwDestroyWindow(this->glfWindow);
    glfwTerminate();
  }
  return 0;
}

void glWindow::setupEnv() {
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
  glUniformBlockBinding(this->shaderProgram->id(),uniformBlockIndexLights, 0);

  glGenBuffers(1, &this->uboLights);
  glBindBuffer(GL_UNIFORM_BUFFER, this->uboLights);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(glLight) * glLight::count, NULL, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, uniformBlockIndexLights, this->uboLights);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  glBindBufferBase(GL_UNIFORM_BUFFER, 1, this->uboLights);
}

void glWindow::render_system(
    Resource<cevy::engine::Window> win, Query<Camera> cams,
    Query<option<Transform>, Handle<Model>, option<Handle<PbrMaterial>>, option<Color>> models,
    Query<option<Transform>, cevy::engine::PointLight> lights,
    cevy::ecs::EventWriter<cevy::ecs::AppExit> close,
    cevy::ecs::World &world) {
  auto atmo = world.get_resource<cevy::engine::Atmosphere>();

  win.get()->render(cams, atmo, models, lights, close);
}

void glWindow::render(
    Query<Camera> cams,
    std::optional<ref<cevy::engine::Atmosphere>> atmosphere,
    Query<option<Transform>, Handle<Model>, option<Handle<PbrMaterial>>, option<Color>> models,
    Query<option<Transform>, cevy::engine::PointLight> lights,
    cevy::ecs::EventWriter<cevy::ecs::AppExit> close) {

  glfwSwapBuffers(this->glfWindow);
  glfwPollEvents();

  auto atmo = atmosphere.has_value() ? atmosphere->get() : cevy::engine::Atmosphere();

  auto fog = filmicToneMapping(atmo.fog.as_vec());
  glClearColor(fog.r, fog.g, fog.b, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  this->shaderProgram->use();

  if (glfwWindowShouldClose(this->glfWindow)) {
    close.send(cevy::ecs::AppExit());
    return;
  }

  if (cams.size() == 0) {
    return;
  }

  auto &camera = std::get<Camera &>(*cams.get_single());

  auto view = glm::scale(camera.projection, glm::vec3(1, float(this->width) / this->height, 1)) *
              camera.view;

  std::vector<glLight> light_buffer;
  light_buffer.clear();
  light_buffer.reserve(glLight::count);

  for (auto [o_tm, light] : lights) {
    if (light_buffer.size() >= glLight::count) break;
    light_buffer.push_back(glLight(light, o_tm.has_value() ? o_tm->position : glm::vec3()));
  }

  glBindBuffer(GL_UNIFORM_BUFFER, this->uboLights);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, light_buffer.size() * sizeof(glLight), light_buffer.data());
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  glUniform1i(this->shaderProgram->uniform("activeLights"), light_buffer.size());
  // std::cout << "activeLights: " << light_buffer.size() << std::endl;

  glUniform3fv(this->shaderProgram->uniform("fog"), 1, glm::value_ptr(atmo.fog.as_vec()));

  glUniform1f(this->shaderProgram->uniform("fog_far"), atmo.fog_distance); // or is it camera.far?

  glUniformMatrix4fv(this->shaderProgram->uniform("view"), 1, GL_FALSE, glm::value_ptr(view));

  auto invView = glm::inverse(camera.view);
  invView = invView / invView[3][3];

  glUniformMatrix4fv(this->shaderProgram->uniform("invView"), 1, GL_FALSE, glm::value_ptr(invView));

  // std::cout << "rendering " << models.size() << " models" << std::endl;
  for (auto [o_tm, h_model, o_h_material, o_color] : models) {
    auto tm = o_tm ? o_tm->mat4() : glm::mat4(1);
    auto model = h_model.get();
    glm::vec4 white = glm::vec4(1, 1, 1, 1);
    auto &color = o_color ? o_color.value().as_vec() : white;
    PbrMaterial &material = o_h_material ? *o_h_material->get() : this->defaultMaterial;

    glUniform3fv(this->shaderProgram->uniform("ambientColor"), 1,
                 glm::value_ptr(atmo.ambiant.as_vec().xyz() + material.ambiant));
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

void glWindow::updateSize(int width, int height) {
  this->width = width;
  this->height = height;
}

void glWindow::keyInput(int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(this->glfWindow, GLFW_TRUE);
}

void glWindow::cursor(double xpos, double ypos) {}

void glWindow::mouseInput(int button, int action, int mods) {}

bool glWindow::init_context() {
  if (!glfwInit()) {
    throw std::runtime_error("failed to init glfw");
    // Initialization failed
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  this->glfWindow = glfwCreateWindow(width, height, "C++evy glWindow", NULL, NULL);
  if (!this->glfWindow) {
    glfwTerminate();
    throw std::runtime_error("failed to create window");
  }
  glfwSetWindowUserPointer(this->glfWindow, this);
  glfwMakeContextCurrent(this->glfWindow);

  glfwSetWindowSizeCallback(this->glfWindow, [](GLFWwindow *win, int width, int height) {
    getFromWin(win)->updateSize(width, height);
  });
  glfwSetMouseButtonCallback(this->glfWindow,
                             [](GLFWwindow *win, int button, int action, int mods) {
                               getFromWin(win)->mouseInput(button, action, mods);
                             });
  glfwSetCursorPosCallback(this->glfWindow, [](GLFWwindow *win, double xpos, double ypos) {
    getFromWin(win)->cursor(xpos, ypos);
  });
  glfwSetKeyCallback(this->glfWindow,
                     [](GLFWwindow *win, int key, int scancode, int action, int mods) {
                       getFromWin(win)->keyInput(key, scancode, action, mods);
                     });

#if _WIN32
  if (gl3wInit()) {
    fprintf(stderr, "failed to initialize OpenGL\n");
    return -1;
  }
  if (!gl3wIsSupported(4, 2)) {
    fprintf(stderr, "OpenGL 4.2 not supported\n");
    return -1;
  }
#elif __linux__
  if (glewInit()) {
    // std::string err(glewGetErrorString());
    throw std::runtime_error("failed to init glew");

    // fprintf(stderr, "failed to initialize OpenGL\n");
    return -1;
  }
  if (!glewIsSupported("GL_VERSION_4_2")) {
    throw std::runtime_error("glew unsupported");
    // fprintf(stderr, "OpenGL 4.2 not supported\n");
    return -1;
  }
#endif // _WIN32

  printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
  glfwSwapInterval(1); // enable vsync
  return 0;
}

bool glWindow::unload_context() {
  glfwTerminate();
  return 0;
}