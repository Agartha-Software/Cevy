/*
** AgarthaSoftware, 2024
** Cevy
** File description:
** Shadow mapping logic
*/

#pragma once

#if (_WIN32)
#include <GL/gl3w.h>
#endif
#if (__linux__)
#include <GL/glew.h>
#endif

#include <glm/vec2.hpp>
#include <utility>

class ShadowMap {
  public:
  ShadowMap(int width = 1024, int height = 1024) : width(width), height(height) {};
  ShadowMap(ShadowMap &&other) { *this = std::move(other); };
  ShadowMap &operator=(ShadowMap &&other) {
    this->deinit();
    this->framebuffer = other.framebuffer;
    other.framebuffer = 0;
    this->shadowMap = other.shadowMap;
    other.shadowMap = 0;
    this->height = other.height;
    this->width = other.width;
    return *this;
  }
  ~ShadowMap() { this->deinit(); }
  void init() {
    glGenFramebuffers(1, &this->framebuffer);

    glGenTextures(1, &this->shadowMap);
    glBindTexture(GL_TEXTURE_2D, this->shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, this->width, this->height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->shadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
  }

  void deinit() {
    glDeleteFramebuffers(1, &this->framebuffer);
    this->framebuffer = 0;
    glDeleteTextures(1, &this->shadowMap);
    this->shadowMap = 0;
  }

  void write() { glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->framebuffer); }
  void read(uint8_t attachment_n) const {
    glActiveTexture(GL_TEXTURE0 + attachment_n);
    glBindTexture(GL_TEXTURE_2D, this->shadowMap);
  };

  inline constexpr glm::vec<2, uint32_t> size() const { return {this->width, this->height}; }

  protected:
  GLuint framebuffer;
  GLuint shadowMap;

  uint32_t width;
  uint32_t height;
};
