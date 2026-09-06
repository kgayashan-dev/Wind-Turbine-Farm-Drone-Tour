// Movement functions.
#include "windfarm/Animation.hpp"

// Angle conversion.
#include <glm/trigonometric.hpp>

namespace windfarm {

// Update state in place; deltaTime is elapsed time in seconds.
// Move objects for this frame.
void updateAnimation(SceneState &state, float deltaTime) {
  // Stop movement when paused.
  if (state.paused) {
    return;
  }

  // Move the drone.
  if (state.droneMoving) {
    state.droneProgress += state.droneSpeed * deltaTime;
  }

  // The same angle drives every turbine rotor.
  if (state.turbinesMoving) {
    state.bladeDegrees += state.turbineSpeed * deltaTime;
  }

  // Move the vehicle and turn its wheels.
  if (state.vehicleMoving) {
    state.vehicleX += state.vehicleSpeed * deltaTime;
    state.wheelDegrees -=
        glm::degrees((state.vehicleSpeed * deltaTime) / kWheelRadius);

    // Re-enter at the left after leaving the right side of the farm.
    if (state.vehicleX > 36.0f) {
      state.vehicleX = -36.0f;
    }
  }

  // Slowly rotate the overview camera when that mode is selected.
  if (state.cameraMode == 1) {
    state.overviewAngle += 0.12f * deltaTime;
  }
}

glm::vec3 vehiclePosition(const SceneState &state) {
  return {state.vehicleX, 0.25f, 0.0f};
}

} // namespace windfarm
