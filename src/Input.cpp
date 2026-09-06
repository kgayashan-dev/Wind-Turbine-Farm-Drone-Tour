#include "windfarm/Input.hpp"

#include <OpenGL/gl3.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <imgui.h>

#include <algorithm>

namespace windfarm {
namespace {

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
  auto *context = static_cast<AppContext *>(glfwGetWindowUserPointer(window));
  if (context == nullptr || context->window == nullptr) {
    return;
  }

  context->window->width = std::max(width, 1);
  context->window->height = std::max(height, 1);
  glViewport(0, 0, context->window->width, context->window->height);
}

void keyCallback(GLFWwindow *window, int key, int, int action, int) {
  if (action != GLFW_PRESS) {
    return;
  }

  // Do not activate scene shortcuts while the UI is using the keyboard.
  if (ImGui::GetCurrentContext() != nullptr &&
      ImGui::GetIO().WantCaptureKeyboard) {
    return;
  }

  auto *context = static_cast<AppContext *>(glfwGetWindowUserPointer(window));
  if (context == nullptr || context->scene == nullptr) {
    return;
  }
  SceneState &state = *context->scene;

  switch (key) {
  case GLFW_KEY_ESCAPE:
    glfwSetWindowShouldClose(window, GLFW_TRUE);
    break;
  case GLFW_KEY_SPACE:
    state.paused = !state.paused;
    break;
  case GLFW_KEY_C:
    state.cameraMode = (state.cameraMode + 1) % 3;
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

void installInputCallbacks(GLFWwindow *window, AppContext *context) {
  glfwSetWindowUserPointer(window, context);
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
  glfwSetKeyCallback(window, keyCallback);
}

void processContinuousInput(GLFWwindow *window, SceneState &state,
                            float deltaTime) {
  if (ImGui::GetIO().WantCaptureKeyboard) {
    return;
  }

  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    state.overviewAngle -= 0.85f * deltaTime;
  }
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    state.overviewAngle += 0.85f * deltaTime;
  }
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    state.overviewHeight += 7.0f * deltaTime;
  }
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
