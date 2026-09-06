// Include this header only once per compilation unit to avoid duplicate definitions.
#pragma once

// GLM vector and matrix types used for positions, directions, and transforms.
#include <glm/glm.hpp>

namespace windfarm {

// All values that can change while the program is running.
struct SceneState {
  // Switches control automatic motion and optional rendering features.
  bool paused = false;
  bool droneMoving = true;
  bool turbinesMoving = true;
  bool vehicleMoving = true;
  bool shadows = true;
  bool lighting = true;
  bool axes = true;

  // 0 = drone route, 1 = overview orbit, 2 = vehicle follow.
  int cameraMode = 0;

  // Route/orbit angles use radians; blade and wheel rotations use degrees.
  // Positions, heights, and radii use the scene's world units.
  float droneProgress = 0.3f;
  float overviewAngle = 0.65f;
  float overviewRadius = 42.0f;
  float overviewHeight = 19.0f;
  float bladeDegrees = 0.0f;
  float vehicleX = -34.0f;
  float wheelDegrees = 0.0f;

  // Animation rates per second: route progress, blade degrees, and vehicle world units.
  float droneSpeed = 0.23f;
  float turbineSpeed = 72.0f;
  float vehicleSpeed = 6.2f;

  // The visible sun and lighting/shadow calculations share this world-space position.
  glm::vec3 sunPosition{-34.0f, 24.0f, -22.0f};
};

// Current framebuffer size in pixels. Retina displays may report a size that
// differs from the logical window size.
struct WindowState {
  int width = 1280;
  int height = 720;
};

// Shared wheel size keeps rendered wheels and distance-to-rotation calculations consistent.
inline constexpr float kWheelRadius = 0.38f;

} // namespace windfarm
