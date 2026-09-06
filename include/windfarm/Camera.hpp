#pragma once

#include "windfarm/AppState.hpp"

#include <glm/glm.hpp>

namespace windfarm {

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
