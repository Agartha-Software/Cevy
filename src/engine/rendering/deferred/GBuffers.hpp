/*
** Agartha-Software, 2024
** C++evy
** File description:
** Deferred G buffer encapsulation
*/

#include "glx.hpp"

#include <algorithm>
#include <iostream>
#include <sys/types.h>
#include <vector>

class GBuffers {
  public:
  struct gbuffer_spec {
    uint32_t attachment = 0;
    int format = -1;
    int filter = -1;
    int active = 0;
  };
  GBuffers(uint32_t width = 0, uint32_t height = 0) : width(width), height(height) {
    std::cout << " <<<< GBuffers(width, height) @" << this << "  <<<<" << std::endl;
  };

  GBuffers(GBuffers &&rhs) {
    std::cout << " <<<< GBuffers(GBuffers) @" << this << "  <<<<" << std::endl;

    this->initted = rhs.initted;
    this->textures = std::move(rhs.textures);
    this->specs = std::move(rhs.specs);
    this->attachments = std::move(rhs.attachments);
    this->framebuffer = rhs.framebuffer;
    this->depthbuffer = rhs.depthbuffer;
    rhs.clear();
  }

  void clear() {
    this->attachments.clear();
    this->textures.clear();
    this->specs.clear();
    this->initted = false;
    this->depthbuffer = 0;
    this->framebuffer = 0;
  }

  ~GBuffers() {
    std::cout << " <<<< ~GBuffers @" << this << "  <<<<" << std::endl;

    if (!initted)
      return;
    for (auto &tex : this->textures) {
      glDeleteTextures(1, &tex);
      tex = 0;
    }
    this->textures.clear();
    this->attachments.clear();
    this->specs.clear();
    glDeleteRenderbuffers(1, &this->depthbuffer);
    glDeleteFramebuffers(1, &this->framebuffer);
    this->initted = false;
  }

  void init() {
    if (this->initted)
      return;

    GLint64 get;
    glGetInteger64v(GL_MAX_COLOR_ATTACHMENTS, &get);
    this->MAX_SIZE = get;
    std::cout << "MAX ATTACH SIZE = " << this->MAX_SIZE << std::endl;

    glGenFramebuffers(1, &this->framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer);

    glGenTextures(1, &this->depthbuffer);
    glBindTexture(GL_TEXTURE_2D, this->depthbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, this->width, this->height, 0,
                 GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                           this->depthbuffer, 0);

    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
      printf("FB error, status: 0x%x\n", Status);
      // throw "framebuffer gen";
    }

    this->initted = true;
  }

  void init_default() {
    init();

    add_spec(0, GL_RGBA16F, GL_NEAREST);
    add_spec(1, GL_RGBA16F, GL_NEAREST);
    add_spec(2, GL_RGBA16F, GL_NEAREST);
    add_spec(3, GL_RGBA, GL_NEAREST);
    add_spec(4, GL_RGBA, GL_NEAREST);
    rebuild();
  }

  void add(uint32_t attachment, int format, int filter) {
    if (attachment >= MAX_SIZE)
      return;

    int index = add_spec(attachment, format, filter);
    if (index >= 0)
      this->build(index);
    glDrawBuffers(this->attachments.size(), &this->attachments[0]);
  }

  int add_spec(uint32_t attachment, int format, int filter) {
    if (attachment >= MAX_SIZE)
      return -1;

    auto found = std::find_if(this->specs.begin(), this->specs.end(),
                              [attachment](auto spec) { return spec.attachment == attachment; });
    int index;
    if (found != this->specs.end()) {
      index = found - this->specs.begin();
      glDeleteTextures(1, &this->textures[index]);
      *found = {.attachment = attachment, .format = format, .filter = filter, .active = 1};
    } else {
      index = this->specs.size();
      this->specs.push_back(
          {.attachment = attachment, .format = format, .filter = filter, .active = 1});
      this->textures.push_back(0);
      this->attachments.push_back(GL_COLOR_ATTACHMENT0 + attachment);
    }
    return index;
  }

  void remove(uint32_t attachment) {
    if (attachment >= MAX_SIZE)
      return;
    auto found = std::find_if(this->specs.begin(), this->specs.end(),
                              [attachment](auto spec) { return spec.attachment == attachment; });
    size_t index;
    if (found != this->specs.end()) {
      index = found - this->specs.begin();
      glDeleteTextures(1, &this->textures[index]);
      this->textures.erase(this->textures.begin() + index);
      this->specs.erase(this->specs.begin() + index);
      this->attachments.erase(this->attachments.begin() + index);
    }
    glDrawBuffers(this->attachments.size(), &this->attachments[0]);
  }

  void rebuild() {
    for (size_t i = 0; i < this->specs.size(); ++i) {
      this->build(i);
      std::cout << "built " << i << ": (" << this->specs[i].attachment << ", " << this->textures[i]
                << ")" << std::endl;
    }
    glDrawBuffers(this->attachments.size(), &this->attachments[0]);
  }

  void read() const {
    for (uint32_t i = 0; i < this->textures.size(); ++i) {
      const auto texture = this->textures[i];
      const auto attachment_n = this->specs[i].attachment;
      if (texture) {
        glActiveTexture(GL_TEXTURE0 + attachment_n);
        glBindTexture(GL_TEXTURE_2D, texture);
      }
    }
  }

  void write() const { glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->framebuffer); }

  uint32_t getFramebuffer() const { return this->framebuffer; }

  uint32_t getDepthbuffer() const { return this->depthbuffer; }

  protected:
  void build(size_t index) {
    auto &spec = this->specs[index];

    GLuint *texture = &this->textures[index];
    glDeleteTextures(1, texture);

    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexImage2D(GL_TEXTURE_2D, 0, spec.format, this->width, this->height, 0, GL_RGBA, GL_FLOAT,
                 NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, spec.filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, spec.filter);
    glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + spec.attachment, GL_TEXTURE_2D,
                           *texture, 0);
  }

  size_t MAX_SIZE = 0;

  uint32_t width;
  uint32_t height;

  uint32_t framebuffer = 0;
  uint32_t depthbuffer = 0;

  std::vector<GLuint> textures;
  std::vector<GLenum> attachments;
  std::vector<gbuffer_spec> specs;

  bool initted = false;
};
