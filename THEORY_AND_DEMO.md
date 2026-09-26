# Wind farm — theory and demonstration

## Short introduction

“This is a real-time 3D wind-energy farm created with modern OpenGL. The scene
demonstrates primitive modelling, hierarchical transformations, rotation and
translation animation, multiple cameras, lighting, long shadows and depth testing.”

## Theory demonstrated

- **Coordinates:** The ground is the X-Z plane and `+Y` is height. Turbine and
  vehicle positions are stored in world coordinates.
- **Modelling:** Frustums form tapered towers, cubes form nacelles, roads and
  buildings, and transformed blade meshes form each rotor.
- **Hierarchy:** A rotor parent matrix is positioned at the turbine hub. Three
  blades inherit this transform and add rotations of 0°, 120° and 240°.
- **Rotation:** Blade angle changes with delta time. Rotation around the local
  Z-axis keeps all blades in the rotor plane.
- **Vehicle motion:** Translation along `+X` is frame-rate independent. Wheels
  rotate according to travelled distance divided by wheel radius.
- **Drone camera:** The camera follows a smooth sinusoidal route between turbine
  rows. `lookAt` builds the view matrix from eye, target and up vectors.
- **Projection:** The shader uses `Projection × View × Model × localPosition`.
- **Lighting:** Normalized normal and light vectors use a dot product for diffuse
  brightness, while ambient light keeps unlit surfaces visible.
- **Shadow mapping:** The scene is first rendered from the sun into a depth
  texture. The camera pass compares each surface with that stored depth, so
  terrain, mountains, buildings, trees, turbines, and the vehicle can receive
  shadows.
- **Visibility:** The z-buffer keeps the nearest fragment for every screen pixel.
- **Interactive interface:** Dear ImGui directly changes animation, camera and
  lighting state at runtime. The live coordinates make world-space movement and
  the relationship between scene parameters and rendered output observable.

## Suggested three-minute demonstration

1. Show the full scene and identify turbines, mountains, vehicle and fixed sun.
2. Press `A` and explain the world-coordinate arrangement.
3. Press `T` to demonstrate rotor hierarchy and rotation.
4. Press `V` to stop/start the vehicle and explain wheel rotation.
5. Press `C` for drone route, overview orbit and vehicle-follow cameras.
6. Press `L` and `H` to isolate lighting and depth-map shadows.
7. Move the sun sliders and explain how the light-space View and Projection
   matrices change the depth map.
8. Explain `T × R × S`, MVP matrices and the z-buffer, then select **Reset scene**.

## Shadow-map quality

A 2048 by 2048 depth texture and 3 by 3 percentage-closer filtering produce
smooth interactive shadows. A small depth bias prevents self-shadowing
artifacts. Like every finite shadow map, very close inspection can still reveal
limited resolution at shadow edges.
