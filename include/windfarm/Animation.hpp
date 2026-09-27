// Read this header once.
#pragma once

// Scene settings.
#include "windfarm/AppState.hpp"

// Vectors and matrices.
#include <glm/glm.hpp>

namespace windfarm { // namespace for the Wind Turbine Farm Drone Tour project

// Advances every moving part by one frame.
void updateAnimation(SceneState &state, float deltaTime); // Update the scene state based on the elapsed time since the last frame.

glm::vec3 vehiclePosition(const SceneState &state);

} // namespace windfarm
