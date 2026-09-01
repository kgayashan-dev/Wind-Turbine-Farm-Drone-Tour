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
  brightness; the half vector produces Blinn–Phong highlights.
- **Long shadows:** The fixed sun is high enough to illuminate the complete scene
  but far to one side, creating a low light angle and long planar shadows.
- **Visibility:** The z-buffer keeps the nearest fragment for every screen pixel.

## Suggested three-minute demonstration

1. Show the full scene and identify turbines, mountains, vehicle and fixed sun.
2. Press `A` and explain the world-coordinate arrangement.
3. Press `T` to demonstrate rotor hierarchy and rotation.
4. Press `V` to stop/start the vehicle and explain wheel rotation.
5. Press `C` for drone route, overview orbit and vehicle-follow cameras.
6. Press `L` and `H` to isolate lighting and the long projected shadows.
7. Explain `T × R × S`, MVP matrices and the z-buffer, then press `R`.

## Limitation

The shadows are projected onto a flat ground plane. Shadow mapping would allow
accurate shadows over uneven terrain and mountain surfaces.
