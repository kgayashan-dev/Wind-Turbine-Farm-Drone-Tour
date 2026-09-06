// Read this header once.
#pragma once

// Vectors and matrices.
#include <glm/glm.hpp>

namespace windfarm {

// All values that can change while the program is running.
struct SceneState {
  // Movement and display switches.
  bool paused = false;
  bool droneMoving = true;
  bool turbinesMoving = true;
  bool vehicleMoving = true;
  bool shadows = true;
  bool lighting = true;
  bool axes = true;

  // 0 = drone route, 1 = overview orbit, 2 = vehicle follow.
  int cameraMode = 0;

  // Positions and angles.
  float droneProgress = 0.3f;
  float overviewAngle = 0.65f;
  float overviewRadius = 42.0f;
  float overviewHeight = 19.0f;
  float bladeDegrees = 0.0f;
  float vehicleX = -34.0f;
  float wheelDegrees = 0.0f;

  // Movement speeds.
  float droneSpeed = 0.23f;
  float turbineSpeed = 72.0f;
  float vehicleSpeed = 6.2f;

  // Sun position.
  glm::vec3 sunPosition{-34.0f, 24.0f, -22.0f};
};

struct WindowState {
  int width = 1280;
  int height = 720;
};

inline constexpr float kWheelRadius = 0.38f;

} // namespace windfarm
