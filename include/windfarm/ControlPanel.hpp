#pragma once

#include "windfarm/AppState.hpp"

#include <glm/glm.hpp>

struct GLFWwindow;

namespace windfarm {

// Dear ImGui lifecycle functions.
bool initializeControlPanel(GLFWwindow *window);
void beginControlPanelFrame();
void drawControlPanel(SceneState &state, const glm::vec3 &cameraPosition,
                      const glm::vec3 &currentVehiclePosition);
void renderControlPanel();
void shutdownControlPanel();

} // namespace windfarm
