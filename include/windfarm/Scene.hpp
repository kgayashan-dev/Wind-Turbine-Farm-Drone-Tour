#pragma once

#include "windfarm/AppState.hpp"
#include "windfarm/Graphics.hpp"

#include <OpenGL/gl3.h>
#include <glm/glm.hpp>

namespace windfarm {

// Owns reusable meshes and draws the complete wind-farm world.
class Scene {
public:
  Scene();
  ~Scene();

  Scene(const Scene &) = delete;
  Scene &operator=(const Scene &) = delete;

  void render(GLuint program, const SceneState &state, const glm::mat4 &view,
              const glm::mat4 &projection,
              const glm::vec3 &cameraPosition) const;

private:
  void drawTerrain(GLuint program) const;
  void drawTurbines(GLuint program, const SceneState &state) const;
  void drawVehicle(GLuint program, const SceneState &state) const;
  void drawAxes(GLuint program) const;

  Mesh cube_;
  Mesh cylinder_;
  Mesh tower_;
  Mesh cone_;
  Mesh sphere_;
};

} // namespace windfarm
