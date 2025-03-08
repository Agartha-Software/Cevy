/*
** Agartha-Software, 2024
** C++evy
** File description:
** primitive generators
*/

#include "Model.hpp"
#include "cevy.hpp"
#include <glm/gtc/constants.hpp>
#include <random>
#include <vector>

namespace cevy::engine::primitives {
Model cube(float size) {
  const std::vector<glm::vec3> vertices = {
      {-size, -size, -size}, {-size, -size, +size}, {-size, +size, +size}, {-size, +size, -size},
      {+size, +size, +size}, {+size, -size, +size}, {+size, -size, -size}, {+size, +size, -size},
      {-size, -size, -size}, {+size, -size, -size}, {+size, -size, +size}, {-size, -size, +size},
      {+size, +size, +size}, {+size, +size, -size}, {-size, +size, -size}, {-size, +size, +size},
      {-size, -size, -size}, {-size, +size, -size}, {+size, +size, -size}, {+size, -size, -size},
      {+size, +size, +size}, {-size, +size, +size}, {-size, -size, +size}, {+size, -size, +size},
  };

  const std::vector<uint32_t> indices = {
      0,  1,  2,  0,  2,  3,  4,  5,  6,  4,  6,  7,  8,  9,  10, 8,  10, 11,
      12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23,
  };

  const auto normals = Model::generate_normals(vertices, indices);

  Model model;
  model.load(vertices, normals, indices);

  return model;
}

Model plane(float size, uint subu, uint subv) {
  std::vector<glm::vec4> vertices;
  std::vector<glm::vec3> colors;
  std::vector<glm::vec3> normals;
  std::vector<uint32_t> indices;

  glm::vec3 colorA = {1, 1, 1};
  glm::vec3 colorB = {0.1, 0.1, 0.1};
  bool colorMode = 0;
  size_t idx = 0;
  auto rdev = std::random_device();
  std::mt19937 gen{rdev()};
  auto d = std::normal_distribution<float>{-1.0, 1.0};

  for (uint u = 0; u < subu; u++) {
    colorMode = u % 2;
    for (uint v = 0; v < subv; v++) {
      vertices.push_back(
          {-size / 2 + float(u - 0) / subu * size, -size / 2 + float(v - 0) / subv * size, 0, 1.0});
      vertices.push_back(
          {-size / 2 + float(u + 1) / subu * size, -size / 2 + float(v - 0) / subv * size, 0, 1.0});
      vertices.push_back(
          {-size / 2 + float(u - 0) / subu * size, -size / 2 + float(v + 1) / subv * size, 0, 1.0});
      vertices.push_back(
          {-size / 2 + float(u + 1) / subu * size, -size / 2 + float(v + 1) / subv * size, 0, 1.0});
      indices.push_back(idx + 0);
      indices.push_back(idx + 1);
      indices.push_back(idx + 2);

      indices.push_back(idx + 2);
      indices.push_back(idx + 1);
      indices.push_back(idx + 3);
      idx += 4;
      colors.push_back(colorMode ? colorA : colorB);
      colors.push_back(colorMode ? colorA : colorB);
      colors.push_back(colorMode ? colorA : colorB);
      colors.push_back(colorMode ? colorA : colorB);
      colorMode = !colorMode;

      auto rdm_x = d(gen) / 10000;
      auto rdm_y = d(gen) / 10000;
      auto nm = glm::normalize(glm::vec3(rdm_x, rdm_y, 1));

      // normals.push_back({0, 0, 1});
      // normals.push_back({0, 0, 1});
      // normals.push_back({0, 0, 1});
      // normals.push_back({0, 0, 1});
      normals.push_back(nm);
      normals.push_back(nm);
      normals.push_back(nm);
      normals.push_back(nm);
    }
  }

  // const auto normals = Model::generate_normals(vertices, indices);

  Model model;
  model.tex_coordinates = cevy::map(vertices, [](const glm::vec3& v) { return glm::vec2{v.x, v.y};});
  model.indices = std::move(indices);
  model.vertices = std::move(vertices);
  model.normals = std::move(normals);
  // model.load(vertices, normals, indices);
  model.colors = std::move(colors);
  model.gl_init();

  return model;
}

Model sphere(float size, uint slices, uint stacks) {
  uint nVerts = (slices + 1) * (stacks + 1);
  uint nIndices = (slices * 2 * (stacks - 1)) * 3;

  std::vector<float> vertices(3 * nVerts);
  std::vector<float> normals(3 * nVerts);
  std::vector<float> uvs(2 * nVerts);
  std::vector<uint> indices(nIndices);

  float theta, phi;
  float thetaFac = glm::two_pi<float>() / slices;
  float phiFac = glm::pi<float>() / stacks;
  float nx, ny, nz, s, t;
  uint idx = 0, tIdx = 0;
  for (uint i = 0; i <= slices; i++) {
    theta = i * thetaFac;
    s = (float)i / slices;
    for (uint j = 0; j <= stacks; j++) {
      phi = j * phiFac;
      t = (float)j / stacks;
      nx = sinf(phi) * cosf(theta);
      ny = sinf(phi) * sinf(theta);
      nz = cosf(phi);
      vertices[idx] = size * nx;
      vertices[idx + 1] = size * ny;
      vertices[idx + 2] = size * nz;
      normals[idx] = nx;
      normals[idx + 1] = ny;
      normals[idx + 2] = nz;
      idx += 3;

      uvs[tIdx] = s;
      uvs[tIdx + 1] = t;
      tIdx += 2;
    }
  }

  // Generate the element list
  idx = 0;
  for (uint i = 0; i < slices; i++) {
    uint stackStart = i * (stacks + 1);
    uint nextStackStart = (i + 1) * (stacks + 1);
    for (uint j = 0; j < stacks; j++) {
      if (j == 0) {
        indices[idx] = stackStart;
        indices[idx + 1] = stackStart + 1;
        indices[idx + 2] = nextStackStart + 1;
        idx += 3;
      } else if (j == stacks - 1) {
        indices[idx] = stackStart + j;
        indices[idx + 1] = stackStart + j + 1;
        indices[idx + 2] = nextStackStart + j;
        idx += 3;
      } else {
        indices[idx] = stackStart + j;
        indices[idx + 1] = stackStart + j + 1;
        indices[idx + 2] = nextStackStart + j + 1;
        indices[idx + 3] = nextStackStart + j;
        indices[idx + 4] = stackStart + j;
        indices[idx + 5] = nextStackStart + j + 1;
        idx += 6;
      }
    }
  }

  Model model;
  model.load(vertices, normals, indices);
  model.gl_init();

  return model;
}

} // namespace cevy::engine::primitives
