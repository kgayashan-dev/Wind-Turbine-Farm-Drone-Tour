# Wind Turbine Farm - Code Structure

The original 907-line program has been separated by responsibility. Start with
`src/main.cpp`; it now reads like a short map of the complete application.

## Recommended demonstration order

1. `src/main.cpp` - application setup and the frame loop.
2. `include/windfarm/AppState.hpp` - every value that changes at runtime.
3. `src/Animation.cpp` - turbine, drone, vehicle and wheel motion.
4. `src/Camera.cpp` - drone, overview and vehicle-follow cameras.
5. `src/Scene.cpp` - terrain, turbines, vehicle, axes, sun and shadows.
6. `src/Graphics.cpp` - primitive meshes, TRS matrices and draw calls.
7. `src/ShaderProgram.cpp` - vertex transformation and pixel lighting.
8. `src/ControlPanel.cpp` - Dear ImGui controls and live coordinates.
9. `src/Input.cpp` - keyboard shortcuts and window resizing.

## Modules

| Module | Purpose |
|---|---|
| `AppState.hpp` | Scene settings, animation values and window size |
| `Animation` | Frame-rate-independent motion |
| `Camera` | Eye and target calculation for three camera modes |
| `Graphics` | Mesh creation, GPU buffers, transforms and drawing |
| `ShaderProgram` | GLSL shader source, compilation and linking |
| `Scene` | Wind-farm model construction and two rendering passes |
| `ControlPanel` | Dear ImGui initialization, widgets and rendering |
| `Input` | GLFW callbacks and held-key camera controls |
| `main.cpp` | High-level initialize -> update -> render -> cleanup flow |

## Frame flow

```text
Poll input
    -> begin the control-panel frame
    -> update animation using delta time
    -> calculate vehicle and camera positions
    -> build UI controls
    -> construct View and Projection matrices
    -> render the 3D scene and shadows
    -> render the UI
    -> swap buffers
```

## Important theory locations

- Translation, rotation and scaling: `Graphics.cpp::makeTransform`
- Hierarchical blade rotation: `Scene.cpp::drawTurbines`
- Vehicle translation and wheel rotation: `Animation.cpp` and
  `Scene.cpp::drawVehicle`
- Camera eye, target and route: `Camera.cpp`
- Model-View-Projection pipeline: vertex shader in `ShaderProgram.cpp`
- Diffuse/specular lighting: fragment shader in `ShaderProgram.cpp`
- Z-buffer and multisampling: OpenGL setup in `main.cpp`
- Planar shadows: shadow projection in `ShaderProgram.cpp` and shadow pass in
  `Scene.cpp::render`
