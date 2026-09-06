// Run the window and scene.
#include "windfarm/Application.hpp"

// Movement functions.
#include "windfarm/Animation.hpp"
// Camera functions.
#include "windfarm/Camera.hpp"
// Control panel functions.
#include "windfarm/ControlPanel.hpp"
// Keyboard and window events.
#include "windfarm/Input.hpp"
// Draw the farm.
#include "windfarm/Scene.hpp"
// Create the shaders.
#include "windfarm/ShaderProgram.hpp"

// OpenGL drawing.
#include <OpenGL/gl3.h>
// OpenGL is included separately.
#define GLFW_INCLUDE_NONE
// Window and input.
#include <GLFW/glfw3.h>

// Move, rotate, and scale.
#include <glm/gtc/matrix_transform.hpp>

// Limit values.
#include <algorithm>
// Console messages.
#include <iostream>

namespace windfarm {

bool Application::initialize() {
  // Start the window library.
  if (!glfwInit()) {
    std::cerr << "GLFW initialization failed.\n";
    return false;
  }

  // Choose the graphics settings.
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
  glfwWindowHint(GLFW_SAMPLES, 4);

  // Open the window.
  window =
      glfwCreateWindow(windowSize.width, windowSize.height,
                       "Wind Turbine Farm Drone Tour", nullptr, nullptr);
  if (window == nullptr) {
    std::cerr << "Window creation failed.\n";
    return false;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Match the screen refresh.

  glfwGetFramebufferSize(window, &windowSize.width, &windowSize.height);
  glViewport(0, 0, windowSize.width, windowSize.height);

  installInputCallbacks(window, &appContext);

  // Prepare drawing and controls.
  shaderProgram = createShaderProgram();
  if (shaderProgram == 0) {
    return false;
  }

  if (!initializeControlPanel(window)) {
    std::cerr << "Dear ImGui initialization failed.\n";
    return false;
  }

  controlPanelReady = true;

  glEnable(GL_DEPTH_TEST);  // Hide objects behind others.
  glEnable(GL_MULTISAMPLE); // Smooth polygon edges.
  glClearColor(0.48f, 0.73f, 0.91f, 1.0f);

  std::cout << "Wind Turbine Farm\n"
            << "C camera | D drone | T turbines | V vehicle | H shadows | "
               "L light\n"
            << "Arrows camera | +/- zoom | A axes | R reset | Space pause\n";

  return true;
}

// Draw one picture.
void Application::drawFrame(const Scene &scene) {
  const glm::vec3 currentVehiclePosition = vehiclePosition(state);
  const CameraFrame camera = calculateCamera(state, currentVehiclePosition);

  drawControlPanel(state, camera.position, currentVehiclePosition);

  // Set the viewing direction.
  const glm::mat4 view = glm::lookAt(camera.position, camera.target,
                                     glm::vec3(0.0f, 1.0f, 0.0f));
  // Fit the picture to the window.
  const float aspectRatio = static_cast<float>(windowSize.width) /
                            static_cast<float>(windowSize.height);
  const glm::mat4 projection =
      glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 140.0f);

  // Draw the world, then the controls.
  scene.render(shaderProgram, state, view, projection, camera.position);
  renderControlPanel();
  glfwSwapBuffers(window);
}

// Repeat until the window closes.
void Application::run() {
  Scene scene;
  double previousTime = glfwGetTime();

  while (!glfwWindowShouldClose(window)) {
    const double currentTime = glfwGetTime();
    // Measure time since the last frame.
    const float deltaTime =
        std::min(static_cast<float>(currentTime - previousTime), 0.05f);
    previousTime = currentTime;

    // Read input and update movement.
    glfwPollEvents();
    beginControlPanelFrame();

    updateAnimation(state, deltaTime);
    processContinuousInput(window, state, deltaTime);

    drawFrame(scene);
  }
}

// Release everything we opened.
void Application::shutdown() {
  if (controlPanelReady) {
    shutdownControlPanel();
    controlPanelReady = false;
  }
  if (shaderProgram != 0) {
    glDeleteProgram(shaderProgram);
    shaderProgram = 0;
  }
  if (window != nullptr) {
    glfwDestroyWindow(window);
    window = nullptr;
  }
  glfwTerminate();
}

} // namespace windfarm
