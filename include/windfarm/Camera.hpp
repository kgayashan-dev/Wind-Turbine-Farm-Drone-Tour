// Include this header only once per compilation unit to avoid duplicate definitions.
#pragma once

// Shared scene and window data definitions; defaults are defined directly in this header.
#include "windfarm/AppState.hpp"

// GLM vector and matrix types used for positions, directions, and transforms.
#include <glm/glm.hpp>

namespace windfarm {

// The camera eye is position; target is the world-space point it looks toward.
struct CameraFrame {
  glm::vec3 position;
  glm::vec3 target;
};

// A smooth mathematical flight path through the turbine rows.
glm::vec3 dronePoint(float progress);

// Calculates the eye and target for the selected camera mode.
CameraFrame calculateCamera(const SceneState &state,
                            const glm::vec3 &currentVehiclePosition);

} // namespace windfarm
