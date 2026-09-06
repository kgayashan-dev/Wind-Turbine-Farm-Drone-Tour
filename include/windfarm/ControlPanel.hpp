// Include this header only once per compilation unit to avoid duplicate definitions.
#pragma once

// Shared scene and window data definitions; defaults are defined directly in this header.
#include "windfarm/AppState.hpp"

// GLM vector and matrix types used for positions, directions, and transforms.
#include <glm/glm.hpp>

// Forward declaration keeps the GLFW implementation dependency in the .cpp file.
struct GLFWwindow;

namespace windfarm {

// Dear ImGui lifecycle functions.
bool initializeControlPanel(GLFWwindow *window);
// Start recording UI widgets for a new frame.
void beginControlPanelFrame();
// Create widgets bound to scene state and display camera/vehicle coordinates.
void drawControlPanel(SceneState &state, const glm::vec3 &cameraPosition,
                      const glm::vec3 &currentVehiclePosition);
// Submit the completed UI to OpenGL after drawing the world.
void renderControlPanel();
// Release UI resources before the window and OpenGL context are destroyed.
void shutdownControlPanel();

} // namespace windfarm
