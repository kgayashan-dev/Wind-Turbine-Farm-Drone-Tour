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
// Absolute value.
#include <cmath>
// Console messages.
#include <iostream>

namespace windfarm
{
  namespace
  {

    // Build an orthographic camera that covers the complete farm from the sun.
    glm::mat4 makeLightSpaceMatrix(const glm::vec3 &sunPosition) // sunposition can be very far away from the farm, so we need to use an orthographic projection to capture the entire scene. The lightSpaceMatrix is used to transform world coordinates into the sun's view space for shadow mapping.
    {
      const glm::vec3 target(0.0f, 3.0f, 0.0f);
      const glm::vec3 direction = glm::normalize(target - sunPosition);
      const glm::vec3 up = std::abs(direction.y) > 0.95f
                               ? glm::vec3(0.0f, 0.0f, 1.0f)
                               : glm::vec3(0.0f, 1.0f, 0.0f);

      // The light's view matrix transforms world coordinates into the sun's view.
      const glm::mat4 lightView = glm::lookAt(sunPosition, target, up);
      const glm::mat4 lightProjection =
          glm::ortho(-60.0f, 60.0f, -60.0f, 60.0f, 1.0f, 180.0f);
      return lightProjection * lightView;
    }

  } // namespace

  bool Application::initialize()
  {
    // Start the window library.
    if (!glfwInit())
    { // window library initialization failed.
      std::cerr << "GLFW initialization failed.\n";
      return false;
    }

    // These are window hints — settings you tell GLFW to apply before creating the window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Use OpenGL 3.3.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Open the window.
    window =
        glfwCreateWindow(windowSize.width, windowSize.height, // Create the window.
                         "Wind Turbine Farm Drone Tour", nullptr, nullptr);
    if (window == nullptr)
    {
      std::cerr << "Window creation failed.\n";
      return false;
    }

    glfwMakeContextCurrent(window); // Make the window's OpenGL context current.
    glfwSwapInterval(1);            // Match the screen refresh.

    glfwGetFramebufferSize(window, &windowSize.width, &windowSize.height);
    glViewport(0, 0, windowSize.width, windowSize.height);

    installInputCallbacks(window, &appContext);

    // Prepare drawing and controls.
    shaderProgram = createShaderProgram();
    if (shaderProgram == 0)
    {
      return false;
    }
    depthShaderProgram = createDepthShaderProgram(); // Create the shader program for the depth rendering from the sun's point of view.
    if (depthShaderProgram == 0)
    {
      return false;
    }
    if (!shadowMap.initialize())
    {
      return false;
    }

    if (!initializeControlPanel(window))
    {
      std::cerr << "ImGui initialization failed.\n";
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
  void Application::drawFrame(const Scene &scene)
  {
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
        glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 140.0f); // The camera is inside the farm, so the near plane is close to the camera.

    const glm::mat4 lightSpaceMatrix = makeLightSpaceMatrix(state.sunPosition);

    // Pass 1: save the scene depth from the sun's point of view.
    if (state.shadows)
    {
      shadowMap.beginDepthPass();
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(2.0f, 4.0f);
      scene.renderDepth(depthShaderProgram, state, lightSpaceMatrix);
      glDisable(GL_POLYGON_OFFSET_FILL);
      shadowMap.endDepthPass(windowSize.width, windowSize.height);
    }

    // Pass 2: draw from the camera and compare with the sun's depth texture.
    shadowMap.bindTexture(GL_TEXTURE0);
    scene.render(shaderProgram, state, view, projection, lightSpaceMatrix);
    renderControlPanel();
    glfwSwapBuffers(window); // Show the picture on the screen.
  }

  // Repeat until the window closes.
  void Application::run()
  { // The main loop that runs until the window is closed.
    Scene scene;
    double previousTime = glfwGetTime(); // Get the current time in seconds.

    while (!glfwWindowShouldClose(window))
    {
      const double currentTime = glfwGetTime();
      // Measure time since the last frame.
      const float deltaTime =
          std::min(static_cast<float>(currentTime - previousTime), 0.05f);
      previousTime = currentTime;

      // Read input and update movement.
      glfwPollEvents();
      beginControlPanelFrame(); // Start a new frame for the control panel.

      updateAnimation(state, deltaTime);
      processContinuousInput(window, state, deltaTime);

      drawFrame(scene);
    }
  }

  // Release everything we opened.
  void Application::shutdown()
  {
    if (controlPanelReady)
    {
      shutdownControlPanel(); //  Release the control panel resources. stop workikng
      controlPanelReady = false;
    }
    if (shaderProgram != 0)
    {
      glDeleteProgram(shaderProgram); // Release the shader program resources.
      shaderProgram = 0;
    }
    if (depthShaderProgram != 0)
    {
      glDeleteProgram(depthShaderProgram);
      depthShaderProgram = 0;
    }
    shadowMap.shutdown(); // Release the shadow map resources.
    // uninstallInputCallbacks(window); // Remove the input callbacks from the window.
    if (window != nullptr)
    {
      glfwDestroyWindow(window); // Release the window resources.
      window = nullptr;
    }
    glfwTerminate();
  }

} // namespace windfarm
