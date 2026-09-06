// Include this header only once per compilation unit to avoid duplicate definitions.
#pragma once

// Shared scene and window data definitions; defaults are defined directly in this header.
#include "windfarm/AppState.hpp"
// Project mesh and drawing declarations; implemented in src/Graphics.cpp.
#include "windfarm/Graphics.hpp"

// OpenGL types and functions for GPU resources, rendering, and graphics state.
#include <OpenGL/gl3.h>
// GLM vector and matrix types used for positions, directions, and transforms.
#include <glm/glm.hpp>

namespace windfarm {

// Owns reusable meshes and draws the complete wind-farm world.
class Scene {
public:
  Scene();
  ~Scene();

  // Disallow copying so two Scene objects cannot delete the same GPU resources.
  Scene(const Scene &) = delete;
  Scene &operator=(const Scene &) = delete;

  // Draw a complete frame using camera matrices supplied by main.cpp.
  void render(GLuint program, const SceneState &state, const glm::mat4 &view,
              const glm::mat4 &projection,
              const glm::vec3 &cameraPosition) const;

private:
  void drawTerrain(GLuint program) const;
  void drawTurbines(GLuint program, const SceneState &state) const;
  void drawVehicle(GLuint program, const SceneState &state) const;
  void drawAxes(GLuint program) const;

  // Owned primitive meshes reused across terrain, turbines, vehicle, and scenery.
  Mesh cube_;
  Mesh cylinder_;
  Mesh tower_;
  Mesh cone_;
  Mesh sphere_;
};

} // namespace windfarm
