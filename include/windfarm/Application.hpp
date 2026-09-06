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
  bool initialize();
  void run();
  void shutdown();

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
