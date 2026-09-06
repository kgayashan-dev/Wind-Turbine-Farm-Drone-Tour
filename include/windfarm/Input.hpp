// Include this header only once per compilation unit to avoid duplicate definitions.
#pragma once

// Shared scene and window data definitions; defaults are defined directly in this header.
#include "windfarm/AppState.hpp"

// Forward declaration: this header needs only a pointer, not the full GLFW API.
struct GLFWwindow;

namespace windfarm {

// The callbacks recover this object through GLFW's window-user pointer.
struct AppContext {
  SceneState *scene = nullptr;
  WindowState *window = nullptr;
};

// Register event callbacks; the context and referenced state must outlive their use.
void installInputCallbacks(GLFWwindow *window, AppContext *context);
// Apply held-key changes each frame, scaled by elapsed seconds.
void processContinuousInput(GLFWwindow *window, SceneState &state,
                            float deltaTime);

} // namespace windfarm
