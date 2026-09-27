// Movement functions.
#include "windfarm/Animation.hpp"

// Angle conversion.
#include <glm/trigonometric.hpp>

namespace windfarm {

// Update state in place; deltaTime is elapsed time in seconds.
// Move objects for this frame.
void updateAnimation(SceneState &state, float deltaTime) { // Update the scene state based on the elapsed time since the last frame.
  // Stop movement when paused.
  if (state.paused) {
    return;
  }

  // Move the drone.
  if (state.droneMoving) { 
    state.droneProgress += state.droneSpeed * deltaTime; // Update the drone's progress along its route based on its speed and the elapsed time.
  }

  // The same angle drives every turbine rotor.
  if (state.turbinesMoving) {
    state.bladeDegrees += state.turbineSpeed * deltaTime; // Update the blade angle based on the turbine speed and the elapsed time.
  }

  // Move the vehicle and turn its wheels.
  if (state.vehicleMoving) {
    state.vehicleX += state.vehicleSpeed * deltaTime; 
    state.wheelDegrees -=
        glm::degrees((state.vehicleSpeed * deltaTime) / kWheelRadius); // Update the wheel angle based on the vehicle speed, elapsed time, and wheel radius.

    // Re-enter at the left after leaving the right side of the farm.
    if (state.vehicleX > 36.0f) {
      state.vehicleX = -36.0f;
    }
  }

  // Slowly rotate the overview camera when that mode is selected.
  if (state.cameraMode == 1) {
    state.overviewAngle += 0.12f * deltaTime; // Update the overview orbital camera angle based on a fixed rotation speed and the elapsed time.
  }
}

glm::vec3 vehiclePosition(const SceneState &state) {
  return {state.vehicleX, 0.25f, 0.0f}; // The vehicle is 0.25 m above the ground.
}

} // namespace windfarm
