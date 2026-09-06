// main.cpp intentionally contains only the high-level application flow.


// Project animation declarations; implemented in src/Animation.cpp (motion and vehicle position).
// These local headers declare the separated functions/classes used below.
// CMake compiles their .cpp implementations and links them into this executable.
// The include/ directory is the search root, so "windfarm/..." resolves there.
#include "windfarm/Animation.hpp"
// Project camera declarations; implemented in src/Camera.cpp (camera position and target).
#include "windfarm/Camera.hpp"
// Project UI declarations; implemented in src/ControlPanel.cpp (Dear ImGui controls).
#include "windfarm/ControlPanel.hpp"
// Project input declarations; implemented in src/Input.cpp (keyboard and resize handling).
#include "windfarm/Input.hpp"
// Project Scene class declaration; implemented in src/Scene.cpp (drawing the farm).
#include "windfarm/Scene.hpp"
// Project shader declaration; implemented in src/ShaderProgram.cpp (GPU program creation).
#include "windfarm/ShaderProgram.hpp"

// OpenGL types and functions for GPU resources, rendering, and graphics state.
#include <OpenGL/gl3.h>
// Prevent GLFW from including another OpenGL header; choose the graphics API explicitly.
#define GLFW_INCLUDE_NONE
// GLFW functions for windows, OpenGL contexts, timing, and keyboard/window events.
#include <GLFW/glfw3.h>

// GLM translation, rotation, scaling, view, and perspective matrix helpers.
#include <glm/gtc/matrix_transform.hpp>

// Standard minimum, maximum, and clamp helpers for limiting numeric values.
#include <algorithm>
// Standard exit status constants, including EXIT_SUCCESS and EXIT_FAILURE.
#include <cstdlib>
// Standard console output streams for instructions and error messages.
#include <iostream>

int main() {
  // Allow project names such as SceneState and updateAnimation without a windfarm:: prefix.
  using namespace windfarm;

  // 1. Create the window and its OpenGL 3.3 Core context.
  if (glfwInit() == GLFW_FALSE) {
    std::cerr << "GLFW initialization failed.\n";
    return EXIT_FAILURE;
  }

  // Request the OpenGL version/profile used by our GLSL shaders and four-sample antialiasing.
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
  glfwWindowHint(GLFW_SAMPLES, 4);

  // Create the default animation/settings state and initial window dimensions.
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

  // Make this window the target of OpenGL calls before creating any GPU resources.
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Vertical synchronization.

  glfwGetFramebufferSize(window, &windowSize.width, &windowSize.height);
  glViewport(0, 0, windowSize.width, windowSize.height);

  // Give callbacks access to these state objects; they remain alive throughout the event loop.
  AppContext appContext{&state, &windowSize};
  installInputCallbacks(window, &appContext);

  // 2. Prepare the shaders, user interface and shared OpenGL settings.
  const GLuint shaderProgram = createShaderProgram();
  // On setup failure, release the resources already created and return an error status.
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
    // Store the clock reading so movement can use elapsed seconds instead of frame counts.
    double previousTime = glfwGetTime();

    // 3. Update and render one complete frame on every loop iteration.
    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
      const double currentTime = glfwGetTime();
      // Limit a frame step to 0.05 seconds to avoid a large animation jump after a stall.
      const float deltaTime =
          std::min(static_cast<float>(currentTime - previousTime), 0.05f);
      previousTime = currentTime;

      // Deliver pending keyboard/resize events, then start a fresh UI frame.
      glfwPollEvents();
      beginControlPanelFrame();

      // Advance moving objects and apply held-key camera controls using the same time step.
      updateAnimation(state, deltaTime);
      processContinuousInput(window, state, deltaTime);

      // Find the vehicle world position, then derive the selected camera eye and target.
      const glm::vec3 currentVehiclePosition = vehiclePosition(state);
      const CameraFrame camera = calculateCamera(state, currentVehiclePosition);

      // Build widgets that edit scene settings and display the calculated coordinates.
      drawControlPanel(state, camera.position, currentVehiclePosition);

      // View transforms world space to camera space. Projection adds depth.
      const glm::mat4 view = glm::lookAt(camera.position, camera.target,
                                         glm::vec3(0.0f, 1.0f, 0.0f));
      // Match the projection to the framebuffer shape so objects do not stretch.
      const float aspectRatio = static_cast<float>(windowSize.width) /
                                static_cast<float>(windowSize.height);
      const glm::mat4 projection =
          glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 140.0f);

      // Draw the 3D world, overlay the UI, and present the completed back buffer.
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
