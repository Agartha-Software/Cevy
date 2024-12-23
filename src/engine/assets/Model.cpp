#define STB_IMAGE_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL

#if (_WIN32)
#include <GL/gl3w.h>
#endif
#if (__linux__)
#include <GL/glew.h>
#endif

#include "Model.hpp"
#include "stb_image.h"
#include <cstdint>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <vector>

using cevy::engine::Model;

Model::Model() {
  this->modelMatrix_ = glm::identity<glm::mat4>();
  this->t_normalMatrix = glm::identity<glm::mat3>();
  this->initialized = false;
}

void Model::load(const std::vector<glm::vec3> &vertices, const std::vector<glm::vec3> &normals,
                 const std::vector<uint32_t> &indices) {
  this->vertices.clear();
  this->vertices.reserve(vertices.size());

  for (auto v = vertices.begin(); v != vertices.end(); v++) {
    this->vertices.push_back({v->x, v->y, v->z, 1.0f});
  }

  this->indices = indices;
  this->normals = normals;
  this->colors = std::vector<glm::vec3>(this->indices.size(), {1, 1, 1});

  std::cout << "v size :" << this->vertices.size() << std::endl;
  std::cout << "i size :" << this->indices.size() << std::endl;

  this->gl_init();
}

void Model::load(const std::vector<glm::vec4> &vertices, const std::vector<glm::vec3> &normals,
                 const std::vector<uint32_t> &indices) {
  this->vertices = vertices;
  this->indices = indices;
  this->normals = normals;
  this->colors = std::vector<glm::vec3>(this->indices.size(), {1, 1, 1});

  std::cout << "v size :" << this->vertices.size() << std::endl;
  std::cout << "i size :" << this->indices.size() << std::endl;

  this->gl_init();
}

void Model::load(const std::vector<float> &vertices, const std::vector<float> &normals,
                 const std::vector<uint32_t> &indices) {
  this->vertices.clear();
  this->vertices.reserve(vertices.size() * 4);

  for (size_t i = 0; i + 2 < vertices.size(); i += 3) {
    this->vertices.push_back({vertices[i + 0], vertices[i + 1], vertices[i + 2], 1.0f});
  }

  this->indices = indices;
  this->normals = std::vector<glm::vec3>((glm::vec3 *)(normals.data()),
                                         (glm::vec3 *)(normals.data() + normals.size()));
  this->colors = std::vector<glm::vec3>(this->vertices.size(), {1, 1, 1});

  std::cout << "v size :" << this->vertices.size() << std::endl;
  std::cout << "i size :" << this->indices.size() << std::endl;

  this->gl_init();
}

void Model::draw() {
  if (!initialized)
    return;

  glBindVertexArray(vaoHandle);
  int size;
  glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

  std::cout << "drawing " << size / sizeof(uint32_t) << " vertices" << std::endl;
  // glDrawElements(GL_LINES_ADJACENCY, size / sizeof(uint32_t), GL_UNSIGNED_INT, 0);
  glDrawElements(GL_TRIANGLES, size / sizeof(uint32_t), GL_UNSIGNED_INT, 0);
}

// void Model::calculate_normals()
// {
// 	this->normals.resize(this->vertices.size());

// 	for (auto i = 0; i < this->indices.size() / 3; ++i) {
// 		const glm::vec3& v0 = this->vertices[this->indices[i * 3]]; //1st vertex
// 		const glm::vec3& v1 = this->vertices[this->indices[i * 3 + 1]]; //2nd vertex
// 		const glm::vec3& v2 = this->vertices[this->indices[i * 3 + 2]]; //3rd vertex
// 		glm::vec3 n = glm::cross((v1 - v0), (v2 - v0)); //Cross product
// 		n = glm::normalize(n);
// 		this->normals[this->indices[i * 3]] += n; //Set the same normal to each vertex
// 		this->normals[this->indices[i * 3 + 1]] += n;
// 		this->normals[this->indices[i * 3 + 2]] += n;
// 	}

// 	for (auto i = 0; i < this->normals.size(); ++i) {
// 		this->normals[i] = glm::normalize(this->normals[i]);
// 	}
// }

std::vector<glm::vec3> Model::generate_normals(const std::vector<glm::vec3> &vertices,
                                               const std::vector<uint32_t> &indices) {
  std::vector<glm::vec3> normals(vertices.size());
  // this->normals.resize(this->vertices.size());

  for (size_t i = 0; i < indices.size() / 3; ++i) {
    const glm::vec3 &v0 = vertices[indices[i * 3]];     // 1st vertex
    const glm::vec3 &v1 = vertices[indices[i * 3 + 1]]; // 2nd vertex
    const glm::vec3 &v2 = vertices[indices[i * 3 + 2]]; // 3rd vertex
    glm::vec3 n = glm::cross((v1 - v0), (v2 - v0));     // Cross product
    n = glm::normalize(n);
    normals[indices[i * 3]] += n; // Set the same normal to each vertex
    normals[indices[i * 3 + 1]] += n;
    normals[indices[i * 3 + 2]] += n;
  }

  for (size_t i = 0; i < normals.size(); ++i) {
    normals[i] = glm::normalize(normals[i]);
    std::cout << glm::to_string(normals[i]) << std::endl;
  }

  return normals;
}

void Model::gl_init() {
  // create VAO : 1 : # of buffer, vaoHandler: pointer for the handler
  glGenVertexArrays(1, &this->vaoHandle);
  glBindVertexArray(this->vaoHandle);

  // create VBO
  glGenBuffers(1, &this->vbo_positions);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo_positions);
  glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(this->vertices[0]),
               this->vertices.data(), GL_STATIC_DRAW);
  // glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPosition), vertexPosition, GL_STATIC_DRAW);
  glVertexAttribPointer(0, // position
                        4, // vector of size 4
                        GL_FLOAT, GL_FALSE,
                        0, // offset
                        0  // stride
  );
  glEnableVertexAttribArray(0);

  glGenBuffers(1, &this->vbo_colors);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo_colors);
  glBufferData(GL_ARRAY_BUFFER, this->colors.size() * sizeof(this->colors[0]), this->colors.data(),
               GL_STATIC_DRAW);
  glVertexAttribPointer(1, // color
                        3, // vector of size 3
                        GL_FLOAT, GL_FALSE,
                        0, // offset
                        0  // stride
  );
  glEnableVertexAttribArray(1);

  glGenBuffers(2, &this->vbo_normals);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo_normals);
  glBufferData(GL_ARRAY_BUFFER, this->normals.size() * sizeof(this->normals[0]),
               this->normals.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(2, // color
                        3, // vector of size 3
                        GL_FLOAT, GL_FALSE,
                        0, // offset
                        0  // stride
  );
  glEnableVertexAttribArray(2);

  if (this->tex_coordinates.size() != 0) {
    glGenBuffers(3, &this->vbo_tex_coordinates);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo_tex_coordinates);
    glBufferData(GL_ARRAY_BUFFER, this->tex_coordinates.size() * sizeof(this->tex_coordinates[0]),
                 this->tex_coordinates.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(3, // color
                          2, // vector of size 3
                          GL_FLOAT, GL_FALSE,
                          0, // offset
                          0  // stride
    );
    glEnableVertexAttribArray(3);
  }

  glGenBuffers(1, &this->ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(this->indices[0]),
               indices.data(), GL_STATIC_DRAW);
  // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glBindVertexArray(0); // deactivate vao

  initialized = true;
}
