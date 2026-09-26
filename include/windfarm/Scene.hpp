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
  Scene(); // Create the meshes for the scene.
  ~Scene(); // Destroy the meshes for the scene.

  Scene(const Scene &) = delete; // 
  Scene &operator=(const Scene &) = delete;

  void render(GLuint program, const SceneState &state, const glm::mat4 &view,
              const glm::mat4 &projection) const;

private:
  void drawTerrain(GLuint program) const; // green ground and mountains
  void drawGround(GLuint program) const; // green ground
  void drawMountains(GLuint program) const; // mountains
  void drawBuilding(GLuint program) const;  // building
  void drawTrees(GLuint program) const;// trees
  void drawTurbines(GLuint program, const SceneState &state) const; // turbines
  void drawVehicle(GLuint program, const SceneState &state) const; // vehicle
  void drawAxes(GLuint program) const; // axes for debugging

  Mesh cube_;
  Mesh cylinder_;
  Mesh tower_;
  Mesh cone_;
  Mesh sphere_;
};

} // namespace windfarm
