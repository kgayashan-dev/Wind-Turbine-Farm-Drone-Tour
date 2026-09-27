// Read this header once.
#pragma once

// Scene settings.
#include "windfarm/AppState.hpp"
// Shapes and drawing.
#include "windfarm/Graphics.hpp"

// OpenGL drawing.
#include <OpenGL/gl3.h>
// Vectors and matrices.
#include <glm/glm.hpp>

namespace windfarm {

// Owns reusable meshes and draws the complete wind-farm world.
class Scene {
public:
  Scene(); // Create the meshes for the farm objects and store them in the Scene object. The meshes are created once and reused for drawing the scene multiple times.
  ~Scene(); // Release the meshes for the farm objects and free their GPU resources. The meshes are destroyed when the Scene object is destroyed.

  Scene(const Scene &) = delete; // Delete the copy constructor to prevent copying of the Scene object, which manages GPU resources and should not be duplicated.
  Scene &operator=(const Scene &) = delete; // Delete the copy assignment operator to prevent copying of the Scene object, which manages GPU resources and should not be duplicated.

  // Draw depth from the sun, then draw the coloured scene from the camera.
  void renderDepth(GLuint depthProgram, const SceneState &state,
                   const glm::mat4 &lightSpaceMatrix) const;
  void render(GLuint program, const SceneState &state, const glm::mat4 &view,
              const glm::mat4 &projection,
              const glm::mat4 &lightSpaceMatrix) const;

private:
  void drawTerrain(GLuint program) const;
  void drawGround(GLuint program) const;
  void drawMountains(GLuint program) const;
  void drawBuilding(GLuint program) const;
  void drawTrees(GLuint program) const;
  void drawTurbines(GLuint program, const SceneState &state) const;
  void drawVehicle(GLuint program, const SceneState &state) const;
  void drawAxes(GLuint program) const;
  void drawShadowCasters(GLuint program, const SceneState &state) const;

  Mesh cube_;
  Mesh cylinder_;
  Mesh tower_;
  Mesh cone_;
  Mesh sphere_;
};

} // namespace windfarm
