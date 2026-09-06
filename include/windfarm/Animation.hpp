// Read this header once.
#pragma once

// Scene settings.
#include "windfarm/AppState.hpp"

// Vectors and matrices.
#include <glm/glm.hpp>

namespace windfarm {

// Advances every moving part by one frame.
void updateAnimation(SceneState &state, float deltaTime);

glm::vec3 vehiclePosition(const SceneState &state);

} // namespace windfarm
