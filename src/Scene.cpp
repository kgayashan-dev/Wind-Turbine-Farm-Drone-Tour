// Project Scene class declaration; implemented in src/Scene.cpp (drawing the farm).
#include "windfarm/Scene.hpp"

// Project animation declarations; implemented in src/Animation.cpp (motion and vehicle position).
#include "windfarm/Animation.hpp"

// GLM translation, rotation, scaling, view, and perspective matrix helpers.
#include <glm/gtc/matrix_transform.hpp>
// glm::value_ptr exposes matrix/vector data for passing values to OpenGL.
#include <glm/gtc/type_ptr.hpp>

// Fixed-size arrays for the six turbine positions.
#include <array>

namespace windfarm {
namespace {

// World-space base positions place three turbines on each side of the road.
constexpr std::array<glm::vec3, 6> kTurbinePositions = {
    glm::vec3{-15.0f, 0.0f, -11.0f}, glm::vec3{0.0f, 0.0f, -14.0f},
    glm::vec3{15.0f, 0.0f, -9.0f},   glm::vec3{-13.0f, 0.0f, 10.0f},
    glm::vec3{3.0f, 0.0f, 7.0f},     glm::vec3{17.0f, 0.0f, 13.0f},
};

} // namespace

// Create reusable primitive meshes once; each scene object is a transformed instance.
Scene::Scene()
    : cube_(createCube()), cylinder_(createCylinder(40)),
      tower_(createCylinder(48, 0.42f)), cone_(createCylinder(40, 0.02f)),
      sphere_(createSphere(20, 32)) {}

// Release owned GPU meshes while main.cpp still has a valid OpenGL context.
Scene::~Scene() {
  destroyMesh(cube_);
  destroyMesh(cylinder_);
  destroyMesh(tower_);
  destroyMesh(cone_);
  destroyMesh(sphere_);
}

// Assemble static scenery from scaled primitives; colours use RGB values from zero to one.
void Scene::drawTerrain(GLuint program) const {
  // Grass field, road and centre markings.
  drawMesh(program, cube_,
           makeTransform({0.0f, -0.25f, 0.0f}, {0.0f, 0.0f, 0.0f},
                         {82.0f, 0.5f, 66.0f}),
           {0.21f, 0.47f, 0.18f});
  drawMesh(program, cube_,
           makeTransform({0.0f, 0.02f, 0.0f}, {0.0f, 0.0f, 0.0f},
                         {76.0f, 0.10f, 4.8f}),
           {0.22f, 0.23f, 0.22f});
  for (int x = -34; x <= 34; x += 7) {
    drawMesh(program, cube_,
             makeTransform({static_cast<float>(x), 0.10f, 0.0f},
                           {0.0f, 0.0f, 0.0f}, {3.4f, 0.025f, 0.16f}),
             {0.91f, 0.82f, 0.28f}, true);
  }

  // Layered cones form the distant mountain range and snowy peaks.
  for (int index = 0; index < 7; ++index) {
    const float x = -32.0f + static_cast<float>(index) * 11.0f;
    const float z = 27.0f + static_cast<float>(index % 2) * 3.0f;
    drawMesh(program, cone_,
             makeTransform({x, 0.0f, z}, {0.0f, 0.0f, 0.0f},
                           {7.5f + static_cast<float>(index % 3),
                            10.5f + static_cast<float>(index % 2) * 2.0f,
                            6.5f + static_cast<float>(index % 2)}),
             {0.31f, 0.38f, 0.30f});
    drawMesh(program, cone_,
             makeTransform({x, 7.3f + static_cast<float>(index % 2) * 1.4f, z},
                           {0.0f, 0.0f, 0.0f}, {2.4f, 3.5f, 2.2f}),
             {0.86f, 0.88f, 0.86f});
  }

  // Farm building and roof.
  drawMesh(program, cube_,
           makeTransform({-27.0f, 2.0f, -23.0f}, {0.0f, 0.0f, 0.0f},
                         {10.0f, 4.0f, 7.0f}),
           {0.58f, 0.45f, 0.28f});
  drawMesh(program, cube_,
           makeTransform({-27.0f, 4.25f, -23.0f}, {0.0f, 0.0f, 0.0f},
                         {11.0f, 0.55f, 8.0f}),
           {0.24f, 0.16f, 0.10f});

  // Cylinders and spheres create rows of trees around the farm.
  for (int index = 0; index < 12; ++index) {
    const float x = -34.0f + static_cast<float>(index) * 6.0f;
    const float z = index % 2 == 0 ? 22.0f : -26.0f;
    drawMesh(
        program, cylinder_,
        makeTransform({x, 0.0f, z}, {0.0f, 0.0f, 0.0f}, {0.15f, 1.3f, 0.15f}),
        {0.29f, 0.15f, 0.06f});
    drawMesh(
        program, sphere_,
        makeTransform({x, 1.7f, z}, {0.0f, 0.0f, 0.0f}, {0.85f, 1.15f, 0.85f}),
        {0.06f, 0.36f, 0.09f});
  }
}

// Build each turbine at its base position, sharing the animated blade angle.
void Scene::drawTurbines(GLuint program, const SceneState &state) const {
  for (std::size_t index = 0; index < kTurbinePositions.size(); ++index) {
    const glm::vec3 position = kTurbinePositions[index];

    // Tower and nacelle.
    drawMesh(program, tower_,
             makeTransform(position, {0.0f, 0.0f, 0.0f}, {0.72f, 10.5f, 0.72f}),
             {0.80f, 0.82f, 0.81f});
    drawMesh(program, cube_,
             makeTransform(position + glm::vec3(0.0f, 10.55f, -0.25f),
                           {0.0f, 0.0f, 0.0f}, {1.25f, 0.72f, 2.0f}),
             {0.72f, 0.75f, 0.75f});

    // The rotor is the parent transform shared by the hub and three blades.
    const glm::mat4 rotor =
        glm::translate(glm::mat4(1.0f),
                       position + glm::vec3(0.0f, 10.55f, -1.32f)) *
        glm::rotate(glm::mat4(1.0f),
                    glm::radians(state.bladeDegrees +
                                 static_cast<float>(index) * 13.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f));

    drawMesh(program, sphere_,
             rotor * makeTransform({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f},
                                   {0.48f, 0.48f, 0.38f}),
             {0.90f, 0.91f, 0.88f});

    // Space three blades 120 degrees apart; each child transform inherits the rotor rotation.
    for (int blade = 0; blade < 3; ++blade) {
      const glm::mat4 bladeParent =
          rotor * glm::rotate(glm::mat4(1.0f),
                              glm::radians(static_cast<float>(blade) * 120.0f),
                              glm::vec3(0.0f, 0.0f, 1.0f));
      drawMesh(program, cube_,
               bladeParent * makeTransform({0.0f, 1.95f, 0.0f},
                                           {0.0f, 0.0f, -4.0f},
                                           {0.30f, 3.9f, 0.15f}),
               {0.88f, 0.89f, 0.86f});
    }

    // Small red aviation warning light behind the rotor.
    drawMesh(program, sphere_,
             makeTransform(position + glm::vec3(0.0f, 10.55f, -1.72f),
                           {0.0f, 0.0f, 0.0f}, {0.12f, 0.12f, 0.12f}),
             {1.0f, 0.08f, 0.04f}, true);
  }
}

void Scene::drawVehicle(GLuint program, const SceneState &state) const {
  // One parent translation moves the complete vehicle along the road.
  const glm::mat4 vehicleParent =
      glm::translate(glm::mat4(1.0f), vehiclePosition(state));

  drawMesh(program, cube_,
           vehicleParent * makeTransform({0.0f, 0.56f, 0.0f},
                                         {0.0f, 0.0f, 0.0f},
                                         {3.0f, 0.62f, 1.45f}),
           {0.94f, 0.48f, 0.05f});
  drawMesh(program, cube_,
           vehicleParent * makeTransform({-0.35f, 1.08f, 0.0f},
                                         {0.0f, 0.0f, 0.0f},
                                         {1.45f, 0.58f, 1.18f}),
           {0.14f, 0.27f, 0.34f});
  drawMesh(program, cube_,
           vehicleParent * makeTransform({1.5f, 0.65f, 0.0f},
                                         {0.0f, 0.0f, 0.0f},
                                         {0.08f, 0.22f, 1.0f}),
           {1.0f, 0.90f, 0.45f}, true);

  // Create four wheels from two longitudinal positions and two sides of the vehicle.
  // Rotate each cylinder onto its axle and apply the animated wheel angle.
  for (float x : {-0.95f, 0.95f}) {
    for (float z : {-0.77f, 0.77f}) {
      const glm::mat4 wheel =
          vehicleParent *
          glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.40f, z)) *
          glm::rotate(glm::mat4(1.0f), glm::radians(state.wheelDegrees),
                      glm::vec3(0.0f, 0.0f, 1.0f)) *
          glm::rotate(glm::mat4(1.0f), glm::radians(90.0f),
                      glm::vec3(1.0f, 0.0f, 0.0f)) *
          glm::scale(glm::mat4(1.0f),
                     glm::vec3(kWheelRadius, 0.28f, kWheelRadius));
      drawMesh(program, cylinder_, wheel, {0.035f, 0.035f, 0.04f});
    }
  }
}

// Draw X in red, Y in green, and Z in blue; emissive colours stay bright without lighting.
void Scene::drawAxes(GLuint program) const {
  const glm::vec3 origin(-36.0f, 0.2f, -27.0f);
  drawMesh(program, cube_,
           makeTransform(origin + glm::vec3(1.5f, 0.0f, 0.0f),
                         {0.0f, 0.0f, 0.0f}, {3.0f, 0.07f, 0.07f}),
           {1.0f, 0.0f, 0.0f}, true);
  drawMesh(program, cube_,
           makeTransform(origin + glm::vec3(0.0f, 1.5f, 0.0f),
                         {0.0f, 0.0f, 0.0f}, {0.07f, 3.0f, 0.07f}),
           {0.0f, 1.0f, 0.0f}, true);
  drawMesh(program, cube_,
           makeTransform(origin + glm::vec3(0.0f, 0.0f, 1.5f),
                         {0.0f, 0.0f, 0.0f}, {0.07f, 0.07f, 3.0f}),
           {0.0f, 0.0f, 1.0f}, true);
}

void Scene::render(GLuint program, const SceneState &state,
                   const glm::mat4 &view, const glm::mat4 &projection,
                   const glm::vec3 &cameraPosition) const {
  // Discard last frame's colours/depth and select the shader program for this frame.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(program);

  // Values shared by all objects in this frame.
  glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE,
                     glm::value_ptr(view));
  glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE,
                     glm::value_ptr(projection));
  glUniform3fv(glGetUniformLocation(program, "lightPosition"), 1,
               glm::value_ptr(state.sunPosition));
  glUniform3fv(glGetUniformLocation(program, "cameraPosition"), 1,
               glm::value_ptr(cameraPosition));
  glUniform1f(glGetUniformLocation(program, "shadowPlaneY"), 0.065f);
  setBoolUniform(program, "lightingEnabled", state.lighting);
  setBoolUniform(program, "shadowMode", false);

  // Normal colour pass.
  drawTerrain(program);
  drawTurbines(program, state);
  drawVehicle(program, state);
  if (state.axes) {
    drawAxes(program);
  }
  drawMesh(program, sphere_,
           makeTransform(state.sunPosition, {0.0f, 0.0f, 0.0f},
                         {0.82f, 0.82f, 0.82f}),
           {1.0f, 0.70f, 0.11f}, true);

  if (!state.shadows) {
    return;
  }

  // Shadow pass: redraw moving models as transparent ground projections.
  setBoolUniform(program, "shadowMode", true);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // Keep depth testing but stop shadows writing depth, allowing transparent projections to blend.
  glDepthMask(GL_FALSE);

  drawTurbines(program, state);
  drawVehicle(program, state);

  // Restore graphics state so subsequent rendering writes depth normally.
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
  setBoolUniform(program, "shadowMode", false);
}

} // namespace windfarm
