// Camera functions.
#include "windfarm/Camera.hpp"

// Maths functions.
#include <cmath>

namespace windfarm {

// Calculate the drone camera's position at a particular point on its route.
glm::vec3 dronePoint(float progress) {
  // Move left and right along the X axis.
  const float x = 18.0f * std::sin(progress);

  // Keep the drone above the farm while making it rise and fall gently.
  const float y = 8.5f + 1.7f * std::sin(2.0f * progress);

  // Move forwards and backwards along the Z axis at half the X-axis frequency.
  const float z = 17.0f * std::sin(0.5f * progress);

  // Return the completed position on the smooth flight path.
  return {x, y, z};
}

// Select the camera position and its look-at target for the active camera mode.
CameraFrame calculateCamera(const SceneState &state,
                            const glm::vec3 &currentVehiclePosition) {
  // Camera mode 0 places the viewer on the moving drone route.
  if (state.cameraMode == 0) {
    // Use the current route position as the camera position.
    const glm::vec3 position = dronePoint(state.droneProgress);

    // Look a short distance ahead on the route and slightly toward the ground.
    const glm::vec3 target =
        dronePoint(state.droneProgress + 0.12f) + glm::vec3(0.0f, -1.0f, 0.0f);

    // Return the drone camera position and viewing target.
    return {position, target};
  }

  // Camera mode 1 shows an orbiting overview of the whole farm.
  if (state.cameraMode == 1) {
    // Calculate a position on a horizontal circle around the farm centre.
    glm::vec3 position{state.overviewRadius * std::cos(state.overviewAngle),
                       state.overviewHeight,
                       state.overviewRadius * std::sin(state.overviewAngle)};

    // Look toward a point above the centre of the farm.
    return {position, {0.0f, 5.0f, 0.0f}};
  }

  // Any other mode uses a third-person camera that follows the vehicle.
  const glm::vec3 position =
      currentVehiclePosition + glm::vec3(-9.0f, 4.5f, 7.0f);

  // Aim ahead of the vehicle so the direction of travel remains visible.
  const glm::vec3 target =
      currentVehiclePosition + glm::vec3(5.0f, 1.0f, 0.0f);

  // Return the vehicle-follow camera position and viewing target.
  return {position, target};
}

} // namespace windfarm
