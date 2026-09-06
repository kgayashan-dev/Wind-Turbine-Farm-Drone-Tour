// Read this header once.
#pragma once

// Scene settings.
#include "windfarm/AppState.hpp"

struct GLFWwindow;

namespace windfarm {

struct AppContext {
  SceneState *scene = nullptr;
  WindowState *window = nullptr;
};

void installInputCallbacks(GLFWwindow *window, AppContext *context);
// Apply held-key changes each frame, scaled by elapsed seconds.
void processContinuousInput(GLFWwindow *window, SceneState &state,
                            float deltaTime);

} // namespace windfarm
