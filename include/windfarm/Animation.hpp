// Include this header only once per compilation unit to avoid duplicate definitions.
#pragma once

// Shared scene and window data definitions; defaults are defined directly in this header.
#include "windfarm/AppState.hpp"

// GLM vector and matrix types used for positions, directions, and transforms.
#include <glm/glm.hpp>

namespace windfarm {

// Advances every moving part by one frame.
void updateAnimation(SceneState &state, float deltaTime);

// Converts the vehicle's one-dimensional X value into a world position.
glm::vec3 vehiclePosition(const SceneState &state);

} // namespace windfarm
