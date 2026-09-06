#include "windfarm/ControlPanel.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cmath>

namespace windfarm {

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

void beginControlPanelFrame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void drawControlPanel(SceneState &state, const glm::vec3 &cameraPosition,
                      const glm::vec3 &currentVehiclePosition) {
  ImGui::SetNextWindowPos(ImVec2(18.0f, 18.0f), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(350.0f, 0.0f), ImGuiCond_FirstUseEver);
  ImGui::Begin("Wind Farm Control Panel", nullptr,
               ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::TextColored(ImVec4(0.45f, 0.90f, 0.55f, 1.0f),
                     "REAL-TIME SCENE CONTROLS");
  ImGui::Separator();

  if (ImGui::Button(state.paused ? "Resume all" : "Pause all",
                    ImVec2(155.0f, 0.0f))) {
    state.paused = !state.paused;
  }
  ImGui::SameLine();
  if (ImGui::Button("Reset scene", ImVec2(155.0f, 0.0f))) {
    state = SceneState{};
  }

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

void renderControlPanel() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void shutdownControlPanel() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

} // namespace windfarm
