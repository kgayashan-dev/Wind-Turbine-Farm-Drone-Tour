// Camera functions.
#include "windfarm/Camera.hpp"

// Maths functions.
#include <cmath>

namespace windfarm {

// Find a point on the flight path.
glm::vec3 dronePoint(float progress) {
  return {18.0f * std::sin(progress), 8.5f + 1.7f * std::sin(2.0f * progress),
          17.0f * std::sin(0.5f * progress)};
}

CameraFrame calculateCamera(const SceneState &state,
                            const glm::vec3 &currentVehiclePosition) {
  // Drone view.
  if (state.cameraMode == 0) {
    // Place the camera on the drone route and look slightly ahead.
    return {dronePoint(state.droneProgress),
            dronePoint(state.droneProgress + 0.12f) +
                glm::vec3(0.0f, -1.0f, 0.0f)};
  }

  // View the whole farm.
  if (state.cameraMode == 1) {
    // Circular orbit around the centre of the complete farm.
    glm::vec3 position{state.overviewRadius * std::cos(state.overviewAngle),
                       state.overviewHeight,
                       state.overviewRadius * std::sin(state.overviewAngle)};
    return {position, {0.0f, 5.0f, 0.0f}};
  }

  // Follow the vehicle.
  return {currentVehiclePosition + glm::vec3(-9.0f, 4.5f, 7.0f),
          currentVehiclePosition + glm::vec3(5.0f, 1.0f, 0.0f)};
}

} // namespace windfarm
