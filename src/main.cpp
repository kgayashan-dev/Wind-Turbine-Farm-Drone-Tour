// Application steps.
#include "windfarm/Application.hpp"
// Success and failure codes.
#include <cstdlib>

int main() {
  windfarm::Application app;

  // Open the window and prepare the scene.
  if (!app.initialize()) {
    app.shutdown();
    return EXIT_FAILURE;
  }

  // Run until the user closes the window.
  app.run();

  // Clean up before leaving.
  app.shutdown();
  return EXIT_SUCCESS;
}
