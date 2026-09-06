// Read this header once.
#pragma once

// OpenGL drawing.
#include <OpenGL/gl3.h>
// Vectors and matrices.
#include <glm/glm.hpp>

namespace windfarm {

// IDs of the GPU objects that store one reusable triangle mesh.
struct Mesh {
  GLuint vao = 0;
  GLuint vbo = 0;
  GLuint ebo = 0;
  GLsizei indexCount = 0;
};

Mesh createCube();
Mesh createCylinder(int segments, float topRadius = 1.0f);
Mesh createSphere(int stacks, int slices);
void destroyMesh(Mesh &mesh);

// Creates a Model matrix in Translate -> Rotate -> Scale order.
glm::mat4 makeTransform(glm::vec3 position, glm::vec3 rotationDegrees,
                        glm::vec3 scale);

void setBoolUniform(GLuint program, const char *name, bool value);
void drawMesh(GLuint program, const Mesh &mesh, const glm::mat4 &model,
              glm::vec3 colour, bool emissive = false);

} // namespace windfarm
