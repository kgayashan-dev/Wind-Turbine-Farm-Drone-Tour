// Read this header once.
#pragma once

// Scene settings.
#include "windfarm/AppState.hpp"

// Vectors and matrices.
#include <glm/glm.hpp>

namespace windfarm {

  
struct CameraFrame { // The eye and target for the camera.
  glm::vec3 position; // The eye position of the camera.
  glm::vec3 target; // The target position of the camera.
};

// A smooth mathematical flight path through the turbine rows.
glm::vec3 dronePoint(float progress);

// Calculates the eye and target for the selected camera mode.
CameraFrame calculateCamera(const SceneState &state,
                            const glm::vec3 &currentVehiclePosition);

} // namespace windfarm
