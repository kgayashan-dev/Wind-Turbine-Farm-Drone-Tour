// Read this header once.
#pragma once

// OpenGL drawing.
#include <OpenGL/gl3.h>
// Vectors and matrices.
#include <glm/glm.hpp>

namespace windfarm {

// IDs of the GPU objects that store one reusable triangle mesh.
struct Mesh { // 
  GLuint vao = 0; // Vertex Array Object ID that stores the vertex attribute configuration.
  GLuint vbo = 0; // Vertex Buffer Object ID that stores the vertex data.
  GLuint ebo = 0; // Element Buffer Object ID that stores the index data. unasigned int indexCount = 0; // Number of indices in the mesh.
  GLsizei indexCount = 0;
};

Mesh createCube(); // Create a unit cube mesh centered at the origin with side length 1.0.
Mesh createCylinder(int segments, float topRadius = 1.0f);
Mesh createSphere(int stacks, int slices); // Create a sphere from latitude and longitude segments.
void destroyMesh(Mesh &mesh); // Destroy the mesh and free its GPU resources. The mesh object is reset to an empty state.

// Creates a Model matrix in Translate -> Rotate -> Scale order.
glm::mat4 makeTransform(glm::vec3 position, glm::vec3 rotationDegrees, // Rotation angles in degrees.
                        glm::vec3 scale); // 

void setBoolUniform(GLuint program, const char *name, bool value); // Set a boolean uniform variable in the shader program. The value is converted to an integer (1 for true, 0 for false) before being sent to the GPU.
void drawMesh(GLuint program, const Mesh &mesh, const glm::mat4 &model,
              glm::vec3 colour, bool emissive = false);

} // namespace wind farm
