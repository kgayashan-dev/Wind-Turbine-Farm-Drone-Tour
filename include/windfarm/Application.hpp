// Read this header once.
#pragma once

// Shared settings and input.
#include "windfarm/Input.hpp"
// OpenGL types.
#include <OpenGL/gl3.h>

namespace windfarm {

class Scene;

// Own the window and its settings.
class Application {
public:
  bool initialize(); // app.initialize() will open the window and prepare the scene.
  void run(); // app.run() will loop until the user closes the window.
  void shutdown();  // app.shutdown() will clean up the window and OpenGL resources.

private:
  void drawFrame(const Scene &scene);

  SceneState state;
  WindowState windowSize;
  AppContext appContext{&state, &windowSize};
  GLFWwindow *window = nullptr;
  GLuint shaderProgram = 0;
  bool controlPanelReady = false;
};

} // namespace windfarm
