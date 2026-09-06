// Project mesh and drawing declarations; implemented in src/Graphics.cpp.
#include "windfarm/Graphics.hpp"

// GLM matrix-inverse helpers used when working with normal transforms.
#include <glm/gtc/matrix_inverse.hpp>
// GLM translation, rotation, scaling, view, and perspective matrix helpers.
#include <glm/gtc/matrix_transform.hpp>
// glm::value_ptr exposes matrix/vector data for passing values to OpenGL.
#include <glm/gtc/type_ptr.hpp>

// Standard mathematical functions such as sine, cosine, and floating-point remainder.
#include <cmath>
// Resizable arrays for generated vertex data and triangle indices.
#include <vector>

namespace windfarm {
namespace {

constexpr float kPi = 3.14159265358979323846f;

// Copies vertex and triangle-index data from CPU memory into GPU buffers.
Mesh uploadMesh(const std::vector<float> &vertices,
                const std::vector<unsigned int> &indices) {
  Mesh mesh;
  mesh.indexCount = static_cast<GLsizei>(indices.size());

  // Allocate a vertex-array object (layout), vertex buffer (data), and index buffer (triangles).
  glGenVertexArrays(1, &mesh.vao);
  glGenBuffers(1, &mesh.vbo);
  glGenBuffers(1, &mesh.ebo);

  glBindVertexArray(mesh.vao);

  // Upload geometry once; GL_STATIC_DRAW indicates the mesh data is reused without frequent changes.
  glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
  glBufferData(GL_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
               vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
               indices.data(), GL_STATIC_DRAW);

  // Each vertex contains position XYZ followed by normal XYZ.
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        reinterpret_cast<void *>(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
  return mesh;
}

} // namespace

Mesh createCube() {
  // A cube needs separate vertices per face because every face has a different
  // normal direction. Each group is: position XYZ, normal XYZ.
  const std::vector<float> vertices = {
      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  -0.5f,
      0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,
      0.0f,  1.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

      0.5f,  -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, -0.5f, -0.5f,
      -0.5f, 0.0f,  0.0f,  -1.0f, -0.5f, 0.5f,  -0.5f, 0.0f,
      0.0f,  -1.0f, 0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,

      -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  -0.5f, -0.5f,
      0.5f,  -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  -1.0f,
      0.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, -1.0f, 0.0f,  0.0f,

      0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  0.5f,  -0.5f,
      -0.5f, 1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 1.0f,
      0.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

      -0.5f, 0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,
      0.5f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 0.0f,
      1.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,

      -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f,
      -0.5f, 0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, 0.5f,  0.0f,
      -1.0f, 0.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
  };

  // Each group of three indices forms a triangle; two triangles cover each cube face.
  const std::vector<unsigned int> indices = {
      0,  1,  2,  2,  3,  0,  4,  5,  6,  6,  7,  4,  8,  9,  10, 10, 11, 8,
      12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20,
  };

  return uploadMesh(vertices, indices);
}

// Build a unit-height cylinder along +Y; a smaller top radius makes a tapered tower or cone.
Mesh createCylinder(int segments, float topRadius) {
  std::vector<float> vertices;
  std::vector<unsigned int> indices;
  // Tilt side normals to match the change in radius from bottom to top.
  const float slope = 1.0f - topRadius;

  // Create pairs of bottom/top vertices around the curved side.
  for (int segment = 0; segment <= segments; ++segment) {
    const float angle =
        2.0f * kPi * static_cast<float>(segment) / static_cast<float>(segments);
    const float cosine = std::cos(angle);
    const float sine = std::sin(angle);
    const glm::vec3 normal = glm::normalize(glm::vec3(cosine, slope, sine));

    vertices.insert(vertices.end(),
                    {cosine, 0.0f, sine, normal.x, normal.y, normal.z,
                     topRadius * cosine, 1.0f, topRadius * sine, normal.x,
                     normal.y, normal.z});
  }

  for (int segment = 0; segment < segments; ++segment) {
    const unsigned int first = static_cast<unsigned int>(2 * segment);
    indices.insert(indices.end(), {first, first + 1, first + 2, first + 1,
                                   first + 3, first + 2});
  }

  // Add one flat circular cap. The direction controls triangle winding.
  auto addCap = [&](float y, float radius, float normalY, bool reverse) {
    const auto centre = static_cast<unsigned int>(vertices.size() / 6);
    vertices.insert(vertices.end(), {0.0f, y, 0.0f, 0.0f, normalY, 0.0f});
    const unsigned int rimStart = centre + 1;

    for (int segment = 0; segment <= segments; ++segment) {
      const float angle = 2.0f * kPi * static_cast<float>(segment) /
                          static_cast<float>(segments);
      vertices.insert(vertices.end(),
                      {radius * std::cos(angle), y, radius * std::sin(angle),
                       0.0f, normalY, 0.0f});
    }

    for (int segment = 0; segment < segments; ++segment) {
      const unsigned int first = rimStart + static_cast<unsigned int>(segment);
      const unsigned int second = first + 1;
      if (reverse) {
        indices.insert(indices.end(), {centre, second, first});
      } else {
        indices.insert(indices.end(), {centre, first, second});
      }
    }
  };

  addCap(0.0f, 1.0f, -1.0f, false);
  addCap(1.0f, topRadius, 1.0f, true);
  return uploadMesh(vertices, indices);
}

// Sample latitude/longitude rings on a unit sphere; each position is also its outward normal.
Mesh createSphere(int stacks, int slices) {
  std::vector<float> vertices;
  std::vector<unsigned int> indices;

  for (int stack = 0; stack <= stacks; ++stack) {
    const float phi =
        kPi * static_cast<float>(stack) / static_cast<float>(stacks);
    for (int slice = 0; slice <= slices; ++slice) {
      const float theta =
          2.0f * kPi * static_cast<float>(slice) / static_cast<float>(slices);
      const glm::vec3 point(std::sin(phi) * std::cos(theta), std::cos(phi),
                            std::sin(phi) * std::sin(theta));
      vertices.insert(vertices.end(),
                      {point.x, point.y, point.z, point.x, point.y, point.z});
    }
  }

  // Join adjacent rings with two triangles per grid cell.
  for (int stack = 0; stack < stacks; ++stack) {
    for (int slice = 0; slice < slices; ++slice) {
      const unsigned int first =
          static_cast<unsigned int>(stack * (slices + 1) + slice);
      const unsigned int below = first + static_cast<unsigned int>(slices + 1);
      indices.insert(indices.end(),
                     {first, below, first + 1, first + 1, below, below + 1});
    }
  }

  return uploadMesh(vertices, indices);
}

// Release all GPU objects owned by this mesh and reset its handles.
void destroyMesh(Mesh &mesh) {
  glDeleteVertexArrays(1, &mesh.vao);
  glDeleteBuffers(1, &mesh.vbo);
  glDeleteBuffers(1, &mesh.ebo);
  mesh = {};
}

glm::mat4 makeTransform(glm::vec3 position, glm::vec3 rotationDegrees,
                        glm::vec3 scale) {
  // Build T * Ry * Rx * Rz * S. With column vectors, scaling acts first and translation last.
  glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
  model = glm::rotate(model, glm::radians(rotationDegrees.y),
                      glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::rotate(model, glm::radians(rotationDegrees.x),
                      glm::vec3(1.0f, 0.0f, 0.0f));
  model = glm::rotate(model, glm::radians(rotationDegrees.z),
                      glm::vec3(0.0f, 0.0f, 1.0f));
  return glm::scale(model, scale);
}

// Send a named shader boolean as an integer; the target program must already be active.
void setBoolUniform(GLuint program, const char *name, bool value) {
  glUniform1i(glGetUniformLocation(program, name), value ? 1 : 0);
}

// Upload this object's transform/material and draw its indexed triangles using the active program.
void drawMesh(GLuint program, const Mesh &mesh, const glm::mat4 &model,
              glm::vec3 colour, bool emissive) {
  glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE,
                     glm::value_ptr(model));

  // Inverse-transpose keeps normals correct after non-uniform scaling.
  const glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
  glUniformMatrix3fv(glGetUniformLocation(program, "normalMatrix"), 1, GL_FALSE,
                     glm::value_ptr(normalMatrix));
  glUniform3fv(glGetUniformLocation(program, "objectColor"), 1,
               glm::value_ptr(colour));
  setBoolUniform(program, "emissive", emissive);

  // Restore this mesh's vertex layout and index buffer, then issue the GPU draw call.
  glBindVertexArray(mesh.vao);
  glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
}

} // namespace windfarm
