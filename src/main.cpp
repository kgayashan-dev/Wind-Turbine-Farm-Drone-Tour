// main.cpp intentionally contains only the high-level application flow.

#include "windfarm/Animation.hpp"
#include "windfarm/Camera.hpp"
#include "windfarm/ControlPanel.hpp"
#include "windfarm/Input.hpp"
#include "windfarm/Scene.hpp"
#include "windfarm/ShaderProgram.hpp"

#include <OpenGL/gl3.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cstdlib>
#include <iostream>

int main() {
  using namespace windfarm;

  // 1. Create the window and its OpenGL 3.3 Core context.
  if (glfwInit() == GLFW_FALSE) {
    std::cerr << "GLFW initialization failed.\n";
    return EXIT_FAILURE;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
  glfwWindowHint(GLFW_SAMPLES, 4);

  SceneState state;
  WindowState windowSize;
  GLFWwindow *window =
      glfwCreateWindow(windowSize.width, windowSize.height,
                       "Wind Turbine Farm Drone Tour", nullptr, nullptr);
  if (window == nullptr) {
    std::cerr << "Window creation failed.\n";
    glfwTerminate();
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Vertical synchronization.

  glfwGetFramebufferSize(window, &windowSize.width, &windowSize.height);
  glViewport(0, 0, windowSize.width, windowSize.height);

  AppContext appContext{&state, &windowSize};
  installInputCallbacks(window, &appContext);

  // 2. Prepare the shaders, user interface and shared OpenGL settings.
  const GLuint shaderProgram = createShaderProgram();
  if (shaderProgram == 0) {
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_FAILURE;
  }

  if (!initializeControlPanel(window)) {
    std::cerr << "Dear ImGui initialization failed.\n";
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_FAILURE;
  }

  glEnable(GL_DEPTH_TEST);  // Z-buffer hidden-surface removal.
  glEnable(GL_MULTISAMPLE); // Smooth polygon edges.
  glClearColor(0.48f, 0.73f, 0.91f, 1.0f);

  std::cout << "Wind Turbine Farm\n"
            << "C camera | D drone | T turbines | V vehicle | H shadows | "
               "L light\n"
            << "Arrows camera | +/- zoom | A axes | R reset | Space pause\n";

  // The Scene constructor uploads all reusable primitive meshes to the GPU.
  {
    Scene scene;
    double previousTime = glfwGetTime();

    // 3. Update and render one complete frame on every loop iteration.
    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
      const double currentTime = glfwGetTime();
      const float deltaTime =
          std::min(static_cast<float>(currentTime - previousTime), 0.05f);
      previousTime = currentTime;

      glfwPollEvents();
      beginControlPanelFrame();

      updateAnimation(state, deltaTime);
      processContinuousInput(window, state, deltaTime);

      const glm::vec3 currentVehiclePosition = vehiclePosition(state);
      const CameraFrame camera = calculateCamera(state, currentVehiclePosition);

      drawControlPanel(state, camera.position, currentVehiclePosition);

      // View transforms world space to camera space. Projection adds depth.
      const glm::mat4 view = glm::lookAt(camera.position, camera.target,
                                         glm::vec3(0.0f, 1.0f, 0.0f));
      const float aspectRatio = static_cast<float>(windowSize.width) /
                                static_cast<float>(windowSize.height);
      const glm::mat4 projection =
          glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 140.0f);

      scene.render(shaderProgram, state, view, projection, camera.position);
      renderControlPanel();
      glfwSwapBuffers(window);
    }
  } // Scene meshes are deleted while the OpenGL context still exists.

  // 4. Release libraries and GPU resources in reverse creation order.
  shutdownControlPanel();
  glDeleteProgram(shaderProgram);
  glfwDestroyWindow(window);
  glfwTerminate();
  return EXIT_SUCCESS;
}
