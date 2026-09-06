// Keyboard and window events.
#include "windfarm/Input.hpp"

// OpenGL drawing.
#include <OpenGL/gl3.h>
// OpenGL is included separately.
#define GLFW_INCLUDE_NONE
// Window and input.
#include <GLFW/glfw3.h>
// Control panel widgets.
#include <imgui.h>

// Limit values.
#include <algorithm>

namespace windfarm {
// Keep callback helpers private to this source file.
namespace {

// GLFW calls this when the drawable pixel size changes.
// Update the drawing area after resizing.
void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
  auto *context = static_cast<AppContext *>(glfwGetWindowUserPointer(window));
  if (context == nullptr || context->window == nullptr) {
    return;
  }

  context->window->width = std::max(width, 1);
  context->window->height = std::max(height, 1);
  glViewport(0, 0, context->window->width, context->window->height);
}

// Handle one key press.
void keyCallback(GLFWwindow *window, int key, int, int action, int) {
  if (action != GLFW_PRESS) {
    return;
  }

  if (ImGui::GetCurrentContext() != nullptr &&
      ImGui::GetIO().WantCaptureKeyboard) {
    return;
  }

  auto *context = static_cast<AppContext *>(glfwGetWindowUserPointer(window));
  if (context == nullptr || context->scene == nullptr) {
    return;
  }
  SceneState &state = *context->scene;


  // Handle the key press.
  switch (key) {
  case GLFW_KEY_ESCAPE:
    glfwSetWindowShouldClose(window, GLFW_TRUE);
    break;
  case GLFW_KEY_SPACE:
    state.paused = !state.paused;
    break;
  case GLFW_KEY_C:
    state.cameraMode = (state.cameraMode + 1) % 3; // Cycle through the three camera modes.
    break;
  case GLFW_KEY_D:
    state.droneMoving = !state.droneMoving;
    break;
  case GLFW_KEY_T:
    state.turbinesMoving = !state.turbinesMoving;
    break;
  case GLFW_KEY_V:
    state.vehicleMoving = !state.vehicleMoving;
    break;
  case GLFW_KEY_H:
    state.shadows = !state.shadows;
    break;
  case GLFW_KEY_L:
    state.lighting = !state.lighting;
    break;
  case GLFW_KEY_A:
    state.axes = !state.axes;
    break;
  case GLFW_KEY_R:
    state = SceneState{};
    break;
  default:
    break;
  }
}

} // namespace

// Connect keyboard and resize events.
void installInputCallbacks(GLFWwindow *window, AppContext *context) {
  glfwSetWindowUserPointer(window, context);
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
  glfwSetKeyCallback(window, keyCallback);
}

// Move the camera while keys are held.
void processContinuousInput(GLFWwindow *window, SceneState &state,
                            float deltaTime) {
  if (ImGui::GetIO().WantCaptureKeyboard) {
    return;
  }

  // Move the overview camera with the arrow keys and zoom with + and -.
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    state.overviewAngle -= 0.85f * deltaTime;
  }
  // Move the overview camera with the arrow keys and zoom with + and -.
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    state.overviewAngle += 0.85f * deltaTime;
  }

  // Move the overview camera with the arrow keys and zoom with + and -.
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    state.overviewHeight += 7.0f * deltaTime;
  }
  // Move the overview camera with the arrow keys and zoom with + and -.
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    state.overviewHeight -= 7.0f * deltaTime;
  }
  if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
    state.overviewRadius -= 8.0f * deltaTime;
  }
  if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
    state.overviewRadius += 8.0f * deltaTime;
  }

  state.overviewHeight = std::clamp(state.overviewHeight, 7.0f, 30.0f);
  state.overviewRadius = std::clamp(state.overviewRadius, 24.0f, 58.0f);
}

} // namespace windfarm
