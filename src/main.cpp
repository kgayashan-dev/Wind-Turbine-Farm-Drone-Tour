// Application steps.
#include "windfarm/Application.hpp"
// Success and failure codes.
#include <cstdlib>

int main() {
<<<<<<< Updated upstream
  windfarm::Application app;

  // Open the window and prepare the scene.
  if (!app.initialize()) {
    app.shutdown();
=======
  // Allow project names such as SceneState and updateAnimation without a windfarm:: prefix.
  using namespace windfarm;
ss
  // 1. Create the window and its OpenGL 3.3 Core context.
  if (glfwInit() == GLFW_FALSE) {
    std::cerr << "GLFW initialization failed.\n";
>>>>>>> Stashed changes
    return EXIT_FAILURE;
  }

  // Run until the user closes the window.
  app.run();

  // Clean up before leaving.
  app.shutdown();
  return EXIT_SUCCESS;
}
