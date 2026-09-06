// Project UI declarations; implemented in src/ControlPanel.cpp (Dear ImGui controls).
#include "windfarm/ControlPanel.hpp"

// Prevent GLFW from including another OpenGL header; choose the graphics API explicitly.
#define GLFW_INCLUDE_NONE
// GLFW functions for windows, OpenGL contexts, timing, and keyboard/window events.
#include <GLFW/glfw3.h>
// Dear ImGui widgets, UI state, styling, and frame management.
#include <imgui.h>
// Connects Dear ImGui to GLFW input and window events.
#include <imgui_impl_glfw.h>
// Renders Dear ImGui draw data using OpenGL 3.
#include <imgui_impl_opengl3.h>

// Standard mathematical functions such as sine, cosine, and floating-point remainder.
#include <cmath>

namespace windfarm {

// Create the UI context, configure its style, and connect its input/rendering backends.
bool initializeControlPanel(GLFWwindow *window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsDark();
  ImGuiStyle &style = ImGui::GetStyle();
  style.WindowRounding = 8.0f;
  style.FrameRounding = 5.0f;
  style.GrabRounding = 5.0f;
  style.WindowBorderSize = 1.0f;

  // Install ImGui callbacks while chaining the application callbacks registered earlier.
  // If a backend fails, undo the initialization steps already completed.
  if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
    ImGui::DestroyContext();
    return false;
  }
  if (!ImGui_ImplOpenGL3_Init("#version 330 core")) {
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    return false;
  }
  return true;
}

// Refresh backend state before recording this frame's widgets.
void beginControlPanelFrame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

// Build the panel each frame; widget pointers let ImGui edit SceneState directly.
void drawControlPanel(SceneState &state, const glm::vec3 &cameraPosition,
                      const glm::vec3 &currentVehiclePosition) {
  ImGui::SetNextWindowPos(ImVec2(18.0f, 18.0f), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(350.0f, 0.0f), ImGuiCond_FirstUseEver);
  ImGui::Begin("Wind Farm Control Panel", nullptr,
               ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::TextColored(ImVec4(0.45f, 0.90f, 0.55f, 1.0f),
                     "REAL-TIME SCENE CONTROLS");
  ImGui::Separator();

  // Button calls return true when activated, triggering pause or a reset to defaults.
  if (ImGui::Button(state.paused ? "Resume all" : "Pause all",
                    ImVec2(155.0f, 0.0f))) {
    state.paused = !state.paused;
  }
  ImGui::SameLine();
  if (ImGui::Button("Reset scene", ImVec2(155.0f, 0.0f))) {
    state = SceneState{};
  }

  // Group movement toggles and speed sliders inside an expandable section.
  if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Checkbox("Drone movement", &state.droneMoving);
    ImGui::SliderFloat("Drone speed", &state.droneSpeed, 0.03f, 0.65f, "%.2f");
    ImGui::Checkbox("Turbine rotation", &state.turbinesMoving);
    ImGui::SliderFloat("Turbine speed", &state.turbineSpeed, 0.0f, 180.0f,
                       "%.0f deg/s");
    ImGui::Checkbox("Vehicle movement", &state.vehicleMoving);
    ImGui::SliderFloat("Vehicle speed", &state.vehicleSpeed, 0.0f, 14.0f,
                       "%.1f units/s");
  }

  // Choose one of three camera modes; orbit controls appear only for overview mode.
  if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
    const char *modes[] = {"Drone route", "Overview orbit", "Vehicle follow"};
    ImGui::Combo("Camera mode", &state.cameraMode, modes, 3);
    if (state.cameraMode == 1) {
      ImGui::SliderFloat("Orbit height", &state.overviewHeight, 7.0f, 30.0f,
                         "%.1f");
      ImGui::SliderFloat("Orbit radius", &state.overviewRadius, 24.0f, 58.0f,
                         "%.1f");
    }
  }

  // Edit rendering switches and the world-space light position.
  if (ImGui::CollapsingHeader("Lighting and shadows",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Checkbox("Enable lighting", &state.lighting);
    ImGui::SameLine();
    ImGui::Checkbox("Enable shadows", &state.shadows);
    ImGui::SliderFloat("Sun X", &state.sunPosition.x, -50.0f, 50.0f, "%.1f");
    ImGui::SliderFloat("Sun height", &state.sunPosition.y, 18.0f, 45.0f,
                       "%.1f");
    ImGui::SliderFloat("Sun Z", &state.sunPosition.z, -50.0f, 50.0f, "%.1f");
  }

  // Display coordinates, the blade angle wrapped to one revolution, and measured frame rate.
  if (ImGui::CollapsingHeader("Live coordinates",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("Camera  X:%6.1f  Y:%5.1f  Z:%6.1f", cameraPosition.x,
                cameraPosition.y, cameraPosition.z);
    ImGui::Text("Vehicle X:%6.1f  Y:%5.1f  Z:%6.1f", currentVehiclePosition.x,
                currentVehiclePosition.y, currentVehiclePosition.z);
    ImGui::Text("Blade angle: %6.1f degrees",
                std::fmod(state.bladeDegrees, 360.0f));
    ImGui::Text("Frame rate: %.1f FPS", ImGui::GetIO().Framerate);
  }

  ImGui::Separator();
  ImGui::Checkbox("Show coordinate axes", &state.axes);
  ImGui::TextDisabled("MVP: clip = Projection x View x Model x vertex");
  ImGui::End();
}

// Finalize widget draw commands and submit them after the 3D scene.
void renderControlPanel() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// Release renderer/input backends before destroying the UI context.
void shutdownControlPanel() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

} // namespace windfarm
