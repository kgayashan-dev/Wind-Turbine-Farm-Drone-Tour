#pragma once

#include "windfarm/AppState.hpp"

#include <glm/glm.hpp>

namespace windfarm {

// Advances every moving part by one frame.
void updateAnimation(SceneState &state, float deltaTime);

// Converts the vehicle's one-dimensional X value into a world position.
glm::vec3 vehiclePosition(const SceneState &state);

} // namespace windfarm
