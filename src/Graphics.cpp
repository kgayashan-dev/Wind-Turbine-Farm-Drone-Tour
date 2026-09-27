// Shapes and drawing.
#include "windfarm/Graphics.hpp"

// Matrix maths.
#include <glm/gtc/matrix_inverse.hpp>
// Move, rotate, and scale.
#include <glm/gtc/matrix_transform.hpp>
// Send maths data to OpenGL.
#include <glm/gtc/type_ptr.hpp>

// Maths functions.
#include <cmath>
// Lists of points and triangles.
#include <vector>


// this code is based on the OpenGL tutorial at https://learnopengl.com/Getting-started/Hello-Triangle
namespace windfarm {
namespace {

constexpr float kPi = 3.14159265358979323846f; // The value of pi, used for generating shapes. 

// Send a shape to the graphics card. GPU 
Mesh uploadMesh(const std::vector<float> &vertices,
                const std::vector<unsigned int> &indices) {
  Mesh mesh;
  mesh.indexCount = static_cast<GLsizei>(indices.size());

  glGenVertexArrays(1, &mesh.vao);
  glGenBuffers(1, &mesh.vbo); // 
  glGenBuffers(1, &mesh.ebo);

  glBindVertexArray(mesh.vao); // 

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

// Make a box.
Mesh createCube() { //  6 faces
  const std::vector<float> vertices = { // . Every vertex contains three position values and three normal values. Vertices are repeated for each face because the faces require different normal directions for correct lighting.
      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  -0.5f, // 
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
// The index array usses OpenGL how to connect the cube’s vertices. Each group of three indices creates one triangle. 
  const std::vector<unsigned int> indices = { // 
0,  1,  2,   2,  3,  0,    // Front
4,  5,  6,   6,  7,  4,    // Back
8,  9, 10,  10, 11,  8,    // Left
12, 13, 14,  14, 15, 12,   // Right
16, 17, 18,  18, 19, 16,   // Top
20, 21, 22,  22, 23, 20    // Bottom
  };

  return uploadMesh(vertices, indices); // 
}

// Make a cylinder or tapered tower.
Mesh createCylinder(int segments, float topRadius) { // class
  std::vector<float> vertices;
  std::vector<unsigned int> indices;
  const float slope = 1.0f - topRadius;

  // Create pairs of bottom/top vertices around the curved side.
  for (int segment = 0; segment <= segments; ++segment) {

    // Compute the angle of the segment around the circle and its sine and cosine.
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

  // Join adjacent pairs of vertices with two triangles per segment.
  for (int segment = 0; segment < segments; ++segment) {
    const unsigned int first = static_cast<unsigned int>(2 * segment);
    indices.insert(indices.end(), {first, first + 1, first + 2, first + 1,
                                   first + 3, first + 2});
  }

  // 
  //a helper function that creates the top or bottom circular face of a cylinder
  auto addCap = [&](float y, float radius, float normalY, bool reverse) { // 
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

    // This loop travels around the cylinder one segment at a time. Every iteration generates one small section of the circular geometry. Increasing the segment count makes the cylinder appear smoother.
    for (int segment = 0; segment < segments; ++segment) { // 
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
  return uploadMesh(vertices, indices); // Upload the mesh data to the GPU.
}

// Make a sphere.
Mesh createSphere(int stacks, int slices) {
  std::vector<float> vertices;
  std::vector<unsigned int> indices; // 

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

  return uploadMesh(vertices, indices); // Upload the mesh data to the GPU.
}

// Free the shape memory.
void destroyMesh(Mesh &mesh) { // call from scene destructor to free the mesh memory
  glDeleteVertexArrays(1, &mesh.vao); 
  glDeleteBuffers(1, &mesh.vbo);
  glDeleteBuffers(1, &mesh.ebo);
  mesh = {};
}

// Set size, rotation, and position.
glm::mat4 makeTransform(glm::vec3 position, glm::vec3 rotationDegrees,
                        glm::vec3 scale) {
  glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
  model = glm::rotate(model, glm::radians(rotationDegrees.y),
                      glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::rotate(model, glm::radians(rotationDegrees.x),
                      glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate the model matrix by the specified rotation angles around the X, Y, and Z axes in degrees. The rotation is applied in the order of Y-axis, X-axis, and then Z-axis, which affects how the object is oriented in 3D space.
  model = glm::rotate(model, glm::radians(rotationDegrees.z),
                      glm::vec3(0.0f, 0.0f, 1.0f));
  return glm::scale(model, scale); // Scale the model matrix by the given scale vector, which adjusts the size of the object in 3D space.
}

// Send an on/off setting to the shader.
void setBoolUniform(GLuint program, const char *name, bool value) { // hide the shadow 
  glUniform1i(glGetUniformLocation(program, name), value ? 1 : 0); // Set the uniform variable in the shader program to 1 if the boolean value is true, or 0 if it is false. This allows the shader to use the boolean value for conditional rendering or other logic.
} 

// Draw one shape.

void drawMesh(GLuint program, const Mesh &mesh, const glm::mat4 &model,
              glm::vec3 colour, bool emissive) {
  glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE,
                     glm::value_ptr(model));

  // Keep lighting correct when stretching shapes.
  const glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
  glUniformMatrix3fv(glGetUniformLocation(program, "normalMatrix"), 1, GL_FALSE,
                     glm::value_ptr(normalMatrix));
  glUniform3fv(glGetUniformLocation(program, "objectColor"), 1,
               glm::value_ptr(colour));
  setBoolUniform(program, "emissive", emissive);

  glBindVertexArray(mesh.vao);
  glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr); // Draw the mesh using the index buffer, which allows for efficient rendering of complex shapes by reusing vertex data. The GL_TRIANGLES mode indicates that the indices define triangles, and GL_UNSIGNED_INT specifies the data type of the indices.
}

} // namespace wind farm
