// Draw the farm.
#include "windfarm/Scene.hpp"

// Movement functions.
#include "windfarm/Animation.hpp"

// Move, rotate, and scale.
#include <glm/gtc/matrix_transform.hpp>
// Send maths data to OpenGL.
#include <glm/gtc/type_ptr.hpp>

// Fixed lists.
#include <array>

namespace windfarm {
namespace {

  // The positions of the turbines in the farm.
constexpr std::array<glm::vec3, 6> kTurbinePositions = {
    glm::vec3{-15.0f, 0.0f, -11.0f}, glm::vec3{0.0f, 0.0f, -14.0f},
    glm::vec3{15.0f, 0.0f, -9.0f},   glm::vec3{-13.0f, 0.0f, 10.0f},
    glm::vec3{3.0f, 0.0f, 7.0f},     glm::vec3{17.0f, 0.0f, 13.0f},
    // glm::vec3{3.0f, 0.0f, -9.0f}, // apperd witout overlappi
};

} // namespace

// The Scene class stores the meshes for the farm and draws them.
Scene::Scene()
    : cube_(createCube()), cylinder_(createCylinder(40)), // Create the meshes for the farm objects.
      tower_(createCylinder(48, 0.42f)), cone_(createCylinder(40, 0.02f)), // The tower is a tapered cylinder, and the cone is a very short cylinder with a small top radius.
      sphere_(createSphere(20, 32)) {}// Create the meshes for the farm objects.

Scene::~Scene() { // Release the meshes for the farm objects.
  destroyMesh(cube_);
  destroyMesh(cylinder_);
  destroyMesh(tower_);
  destroyMesh(cone_);
  destroyMesh(sphere_);
}

// Draw the farm from the sun's point of view into the depth texture.
void Scene::drawTerrain(GLuint program) const {
  drawGround(program); // Draw the grass, road, and road markings.
  drawMountains(program); // Draw the distant mountains and snowy peaks.
  drawBuilding(program); // Draw the farm's building.
  drawTrees(program); // Draw the trees along the farm's edges.
}

// These objects are rendered into the sun's depth texture.
void Scene::drawShadowCasters(GLuint program, const SceneState &state) const {
  drawTerrain(program); // Draw the grass, road, mountains, building, and trees.
  drawTurbines(program, state);
  drawVehicle(program, state);
}

// Draw the grass, road, and road markings.
void Scene::drawGround(GLuint program) const {
  drawMesh(program, cube_,
           makeTransform({0.0f, -0.25f, 0.0f}, {0.0f, 0.0f, 0.0f},
                         {82.0f, 0.5f, 66.0f}),
           {0.21f, 0.47f, 0.18f});// Draw the grass as a large green box.

  drawMesh(program, cube_,
           makeTransform({0.0f, 0.02f, 0.0f}, {0.0f, 0.0f, 0.0f},
                         {76.0f, 0.10f, 4.8f}), 
           {0.22f, 0.23f, 0.22f}); // Draw the road as a long dark gray box on top of the grass.

  for (int x = -34; x <= 34; x += 7) { // Draw the road markings as small yellow boxes on top of the road at regular intervals along the X-axis.
    drawMesh(program, cube_,
             makeTransform({static_cast<float>(x), 0.10f, 0.0f},
                           {0.0f, 0.0f, 0.0f}, {3.4f, 0.025f, 0.16f}),
             {0.91f, 0.82f, 0.28f}, true); // Draw the road markings as small yellow boxes on top of the road. The emissive parameter is set to true to make the markings appear bright and reflective.
  }
}

// Layered cones form the distant mountain range and snowy peaks.
void Scene::drawMountains(GLuint program) const {
  for (int index = 0; index < 7; ++index) { // Draw 7 mountains along the farm's horizon.
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
             {0.86f, 0.88f, 0.86f}); // Draw the snowy peaks. COLORS 
  }
}

// Draw the farm's building as a brown box with a dark roof.
void Scene::drawBuilding(GLuint program) const {
  drawMesh(program, cube_,
           makeTransform({-27.0f, 2.0f, -23.0f}, {0.0f, 0.0f, 0.0f},
                         {10.0f, 4.0f, 7.0f}),
           {0.58f, 0.45f, 0.28f});
  drawMesh(program, cube_,
           makeTransform(
            {-27.0f, 4.25f, -23.0f}, 
            {0.0f, 0.0f, 0.0f},
            {11.0f, 0.55f, 8.0f}),
           {0.24f, 0.16f, 0.10f}); // Draw the roof as a dark brown box on top of the building.
}

void Scene::drawTrees(GLuint program) const {
  for (int index = 0; index < 12; ++index) { // Draw 12 trees along the farm's edges.
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

// A rotor parent matrix makes all three blades inherit the same rotation.
void Scene::drawTurbines(GLuint program, const SceneState &state) const {
  for (std::size_t index = 0; index < kTurbinePositions.size(); ++index) {
    const glm::vec3 position = kTurbinePositions[index];

    drawMesh(program, tower_,
             makeTransform(position, {0.0f, 0.0f, 0.0f},
                           {0.72f, 10.5f, 0.72f}),
             {0.80f, 0.82f, 0.81f});
    drawMesh(program, cube_,
             makeTransform(position + glm::vec3(0.0f, 10.55f, -0.25f),
                           {0.0f, 0.0f, 0.0f}, {1.25f, 0.72f, 2.0f}),
             {0.72f, 0.75f, 0.75f});

    const glm::mat4 rotor =
        glm::translate(glm::mat4(1.0f),
                       position + glm::vec3(0.0f, 10.55f, -1.32f)) *
        glm::rotate(glm::mat4(1.0f),
                    glm::radians(state.bladeDegrees +
                                 static_cast<float>(index) * 13.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f));

    drawMesh(program, sphere_,
             rotor * makeTransform({0.0f, 0.0f, 0.0f},
                                   {0.0f, 0.0f, 0.0f},
                                   {0.48f, 0.48f, 0.38f}),
             {0.90f, 0.91f, 0.88f});

    for (int blade = 0; blade < 3; ++blade) { // Draw the three blades of the turbine.
      const glm::mat4 bladeParent =
          rotor * glm::rotate(glm::mat4(1.0f),
                              glm::radians(static_cast<float>(blade) * 120.0f), // Rotate each blade by 120 degrees around the rotor's axis to position them evenly.
                              glm::vec3(0.0f, 0.0f, 1.0f));

      drawMesh(program, cube_,
               bladeParent * makeTransform({0.0f, 1.95f, 0.0f},
                                           {0.0f, 0.0f, -4.0f},
                                           {0.30f, 3.9f, 0.15f}),  // Transform for each blade
               {0.88f, 0.89f, 0.86f});// Draw each blade as a long thin box extending from the rotor's center. The blades are slightly rotated to give them a realistic tilt.
    }

    drawMesh(program, sphere_,
             makeTransform(position + glm::vec3(0.0f, 10.55f, -1.72f), // Draw the small red light on the turbine's nose cone. The light is positioned slightly in front of the rotor to indicate the turbine's operational status.
                           {0.0f, 0.0f, 0.0f}, {0.12f, 0.12f, 0.12f}),// Draw the small red light on the turbine's nose cone.
             {1.0f, 0.08f, 0.04f}, true); // Draw the red light on the turbine's nose cone. The emissive parameter is set to true to make the light appear bright and glowing.
  }
}

void Scene::drawVehicle(GLuint program, const SceneState &state) const {
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

  for (float x : {-0.95f, 0.95f}) {
    for (float z : {-0.77f, 0.77f}) {
      const glm::mat4 wheel =
          vehicleParent *
          glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.40f, z)) *
          glm::rotate(glm::mat4(1.0f), glm::radians(state.wheelDegrees), // Rotate the wheel around its local x-axis based on the wheelDegrees value from the SceneState, which simulates the wheel's rotation as the vehicle moves.
                      glm::vec3(0.0f, 0.0f, 1.0f)) *
          glm::rotate(glm::mat4(1.0f), glm::radians(90.0f),// Rotate the wheel by 90 degrees around the z-axis to align it correctly with the vehicle's orientation.
                      glm::vec3(1.0f, 0.0f, 0.0f)) *
          glm::scale(glm::mat4(1.0f),
                     glm::vec3(kWheelRadius, 0.28f, kWheelRadius));

      drawMesh(program, cylinder_, wheel, {0.035f, 0.035f, 0.04f}); // Draw the wheel as a dark gray cylinder.
    }
  }
}

void Scene::drawAxes(GLuint program) const { // Draw the three axes as long thin boxes with red, green, and blue colours.
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

// First pass: save the nearest surface depth as seen from the sun.
void Scene::renderDepth(GLuint depthProgram, const SceneState &state,
                        const glm::mat4 &lightSpaceMatrix) const {
  glUseProgram(depthProgram);
  glUniformMatrix4fv(
      glGetUniformLocation(depthProgram, "lightSpaceMatrix"), 1, GL_FALSE,
      glm::value_ptr(lightSpaceMatrix));
  drawShadowCasters(depthProgram, state);
}

// Second pass: draw from the camera and compare with the sun's depth map.
void Scene::render(GLuint program, const SceneState &state,
                   const glm::mat4 &view, const glm::mat4 &projection,
                   const glm::mat4 &lightSpaceMatrix) const {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(program);

  glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE,
                     glm::value_ptr(view));
  glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE,
                     glm::value_ptr(projection));
  glUniformMatrix4fv(
      glGetUniformLocation(program, "lightSpaceMatrix"), 1, GL_FALSE,
      glm::value_ptr(lightSpaceMatrix));
  glUniform3fv(glGetUniformLocation(program, "lightPosition"), 1,
               glm::value_ptr(state.sunPosition));
  glUniform1i(glGetUniformLocation(program, "shadowMap"), 0); // Set the shadow map texture unit to 0, which corresponds to the depth texture generated in the first pass. This allows the shader to access the shadow map for shadow calculations during rendering.
  setBoolUniform(program, "lightingEnabled", state.lighting); // Set the lighting and shadows enabled flags in the shader program based on the SceneState. This allows the shader to conditionally apply lighting and shadow calculations during rendering.
  setBoolUniform(program, "shadowsEnabled", state.shadows); // Set the lighting and shadows enabled flags in the shader program based on the SceneState. This allows the shader to conditionally apply lighting and shadow calculations during rendering.

  drawTerrain(program);
  drawTurbines(program, state);
  drawVehicle(program, state);

  if (state.axes) {
    drawAxes(program);
  }

  drawMesh(program, sphere_,
           makeTransform(state.sunPosition, {0.0f, 0.0f, 0.0f},
                         {0.82f, 0.82f, 0.82f}),
           {1.0f, 0.70f, 0.11f}, true); // Draw the sun as a small yellow sphere in the sky. true means the sun is emissive and does not receive lighting from other objects.
}

} // namespace windfarm
