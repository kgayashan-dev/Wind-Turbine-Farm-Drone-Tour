#pragma once

#include "windfarm/AppState.hpp"

struct GLFWwindow;

namespace windfarm {

// The callbacks recover this object through GLFW's window-user pointer.
struct AppContext {
  SceneState *scene = nullptr;
  WindowState *window = nullptr;
};

void installInputCallbacks(GLFWwindow *window, AppContext *context);
void processContinuousInput(GLFWwindow *window, SceneState &state,
                            float deltaTime);

} // namespace windfarm
