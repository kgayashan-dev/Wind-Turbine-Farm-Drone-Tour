// Read this header once.
#pragma once

// Scene settings.
#include "windfarm/AppState.hpp"

// Vectors and matrices.
#include <glm/glm.hpp>

struct GLFWwindow;

namespace windfarm {

// Dear ImGui lifecycle functions.
bool initializeControlPanel(GLFWwindow *window);
// Start recording UI widgets for a new frame.
void beginControlPanelFrame();
void drawControlPanel(SceneState &state, const glm::vec3 &cameraPosition,
                      const glm::vec3 &currentVehiclePosition);
// Submit the completed UI to OpenGL after drawing the world.
void renderControlPanel();
void shutdownControlPanel();

} // namespace windfarm
