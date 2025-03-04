/*
** Agartha-Software, 2024
** C++evy
** File description:
** Screen space quad rendering
*/

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_SWIZZLE

#include <array>
#include <sys/types.h>
#if (_WIN32)
#include <GL/gl3w.h>
#endif
#if (__linux__)
#include <GL/glew.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

/**
 * @brief view space quad drawing helper
 * draws two camera-aligned triangles
 *
 */
class Billboard {
  protected:
  struct coord_pair {
    glm::vec3 position;
    glm::vec2 coord;
  };
  std::array<coord_pair, 4> verts = {{
      {{-1.0f, +1.0f, 0}, {0.0f, 1.0f}},
      {{-1.0f, -1.0f, 0}, {0.0f, 0.0f}},
      {{+1.0f, +1.0f, 0}, {1.0f, 1.0f}},
      {{+1.0f, -1.0f, 0}, {1.0f, 0.0f}},
  }};

  uint quadVAO = 0;
  uint quadVBO = 0;

  /// opengl context it operates in
  intptr_t glContext;

  bool initted = false;

  public:
  Billboard(){};
  /**
   * @brief initialize openGl data;
   *
   */
  void init(intptr_t context = 0) {
    this->glContext = context;

    glGenVertexArrays(1, &this->quadVAO);
    glGenBuffers(1, &this->quadVBO);
    glBindVertexArray(this->quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(decltype(this->verts)::value_type) * this->verts.size(),
                 &this->verts, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, decltype(coord_pair::position)::length(), GL_FLOAT, GL_FALSE,
                          sizeof(decltype(this->verts)::value_type), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, decltype(coord_pair::coord)::length(), GL_FLOAT, GL_FALSE,
                          sizeof(decltype(this->verts)::value_type),
                          &((coord_pair *)(nullptr))->coord);
    initted = true;
  };

  void worldspace(const glm::mat4 &view, const glm::mat4 projection, const glm::vec3 &center,
                  const glm::vec2 size) {
    glm::vec4 view_center = projection * view * glm::vec4(center, 1);
    view_center.x /= view_center.w;
    view_center.y /= view_center.w;
    // view_center.z = view_center.z / view[3][2];
    glm::vec4 offset = projection * glm::vec4(size, view_center.z, 1);
    offset /= offset.w;
    screenspace(view_center.xy() - offset.xy(), view_center.xy() + offset.xy(), 0);
    // screenspace(view_center.xy() - size, view_center.xy() + size);
  }

  void screenspace(const glm::vec2 &a, const glm::vec2 &b, float z = 0) {
    float min_x = std::min(a.x, b.x);
    float min_y = std::min(a.y, b.y);
    float max_x = std::max(a.x, b.x);
    float max_y = std::max(a.y, b.y);
    z = std::abs(z);

    verts[0].position = {min_x, max_y, z};
    verts[1].position = {min_x, min_y, z};
    verts[2].position = {max_x, max_y, z};
    verts[3].position = {max_x, min_y, z};
    update();
  }

  void draw() {
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
  }

  protected:
  void update() {
    glBindBuffer(GL_ARRAY_BUFFER, this->quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(decltype(verts)::value_type) * verts.size(), &verts,
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
};
