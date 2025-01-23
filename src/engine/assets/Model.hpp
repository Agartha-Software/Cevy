/*
** Agartha-Software, 2024
** C++evy
** File description:
** mesh geometry
*/

#pragma once

#include <algorithm>
#include <cstdint>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <limits>
#include <vector>

typedef uint32_t index_t;

namespace cevy::engine {
class Model {
  public:
  Model();
  void load(const std::vector<glm::vec3> &vertices, const std::vector<glm::vec3> &normals,
            const std::vector<uint32_t> &indices);
  void load(const std::vector<glm::vec4> &vertices, const std::vector<glm::vec3> &normals,
            const std::vector<uint32_t> &indices);
  void load(const std::vector<float> &vertices, const std::vector<float> &normals,
            const std::vector<uint32_t> &indices);
  static std::vector<glm::vec3> generate_normals(const std::vector<glm::vec3> &vertices,
                                                 const std::vector<uint32_t> &indices);
  template <typename... T>
  static void merge_by_distance(std::vector<glm::vec3> &vertices, std::vector<uint32_t> &indices,
                                float merge_distance, std::vector<T> &...cleanups);
  template <typename... T>
  static void trim_geometry(std::vector<glm::vec3> &vertices, std::vector<index_t> &indices,
                            std::vector<T> &...cleanups);

  void draw() const;

  // void calculate_normals();

  const glm::mat4 &modelMatrix() const { return modelMatrix_; }
  // ref<glm::mat4> modelMatrix() {
  // 	std::function<void(const glm::mat4&)> func = [this](const glm::mat4& model)
  // {this->setModelMatrix(model);}; 	return ref<glm::mat4>(modelMatrix_, func);
  // }

  void setModelMatrix(const glm::mat4 &modelMatrix) {
    auto normalized = modelMatrix / modelMatrix[3][3];
    t_normalMatrix = glm::inverse(glm::mat3(normalized));
    modelMatrix_ = normalized;
  }

  const glm::mat3 &tNormalMatrix() const { return t_normalMatrix; }

  void gl_init();

  std::vector<glm::vec4> vertices;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec3> colors;
  std::vector<glm::vec2> tex_coordinates;
  std::vector<index_t> indices;

  protected:
  glm::mat4 modelMatrix_;
  glm::mat3 t_normalMatrix;

  uint vaoHandle;
  uint vbo_positions;
  uint vbo_colors;
  uint vbo_normals;
  uint vbo_tex_coordinates;
  uint ibo;

  bool initialized;
};

template <typename... T>
void Model::trim_geometry(std::vector<glm::vec3> &vertices, std::vector<index_t> &indices,
                          std::vector<T> &...cleanups) {
  std::vector<bool> unused; //[index_t];
  unused.resize(vertices.size(), true);

  std::for_each(indices.begin(), indices.end(),
                [&vertices, &unused, &indices](auto idx) { unused[idx] = false; });

  size_t offset = 0;
  std::vector<size_t> offsets = std::vector<size_t>(vertices.size(), 0);
  // [index_t]

  std::vector<glm::vec3> new_vertices;
  new_vertices.reserve(vertices.size());

  for (index_t i = 0; i < vertices.size(); ++i) {
    offsets[i] = offset;
    if (unused[i]) {
      offset += 1;
    } else {
      new_vertices.push_back(vertices[i]);
      ((cleanups[i - offset] = cleanups[i]), ...);
    }
  }

  std::vector<index_t> new_indices;

  new_indices.resize(indices.size());

  for (size_t i = 0; i < new_indices.size(); ++i) {
    // if (!unused[indices[i]])
    new_indices[i] = (indices[i] - offsets[indices[i]]);
  }

  indices = new_indices;
  vertices = new_vertices;
}

template <typename... T>
void Model::merge_by_distance(std::vector<glm::vec3> &vertices, std::vector<uint32_t> &indices,
                              float merge_distance, std::vector<T> &...cleanups) {
  std::vector<std::vector<float>> distances =
      std::vector<std::vector<float>>(vertices.size(), std::vector<float>(vertices.size(), 0.));
  // [index_t][index_t]

  for (size_t i = 0; i < vertices.size(); ++i) {
    for (size_t j = 0; j < vertices.size(); ++j) {
      distances[i][j] = glm::length(vertices[i] - vertices[j]);
    }
  }

  std::vector<index_t> redirect; //[index_t]
  redirect.resize(vertices.size());

  for (index_t i = 0; i < vertices.size(); ++i) {
    redirect[i] = i;
    float best_dist = std::numeric_limits<float>::infinity();
    for (index_t j = 0; j < vertices.size(); ++j) {
      float distance = distances[i][j];
      if (distance < merge_distance && distance < best_dist && i != j && redirect[j] == j) {
        best_dist = distance;
        redirect[i] = j;
        break;
      }
    }
  }

  for (size_t i = 0; i < indices.size(); ++i) {
    indices[i] = redirect[indices[i]];
  }

  Model::trim_geometry(vertices, indices, cleanups...);
}
class Model;
namespace primitives {
Model cube(float size);
// inline Model cube(float size) { return cube({size, size, size}); };
Model plane(float size, uint subu, uint subv);
Model sphere(float size, uint slices, uint stacks);
} // namespace primitives
} // namespace cevy::engine
