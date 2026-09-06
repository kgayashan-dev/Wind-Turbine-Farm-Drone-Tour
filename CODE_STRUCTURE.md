# Simple branch code guide

Start with `src/main.cpp`. It has three steps:

```text
initialize() -> run() -> shutdown()
```

If setup fails, the program cleans up and exits.

## Where to look

| File | What it does |
|---|---|
| `src/main.cpp` | Starts and stops the app |
| `src/Application.cpp` | Opens the window and runs each frame |
| `include/windfarm/AppState.hpp` | Holds settings, positions, and speeds |
| `src/Animation.cpp` | Moves the drone, blades, and vehicle |
| `src/Camera.cpp` | Chooses where the camera looks |
| `src/Input.cpp` | Handles keys and window resizing |
| `src/ControlPanel.cpp` | Builds the buttons and sliders |
| `src/Scene.cpp` | Draws the farm and shadows |
| `src/Graphics.cpp` | Makes and draws basic shapes |
| `src/ShaderProgram.cpp` | Runs the graphics card's drawing code |
| `CMakeLists.txt` | Builds the program and connects libraries |

Headers in `include/windfarm/` list the functions and classes.
Their matching `.cpp` files contain the code.
The old version is in `legacy/`; it is not part of the build.

## One frame

1. Read keyboard and window events.
2. Start the control panel frame.
3. Update movement.
4. Find the vehicle and camera positions.
5. Build the controls.
6. Set the camera view.
7. Draw the farm and shadows.
8. Draw the controls and show the picture.

`Application::drawFrame()` handles steps 4 to 8.
The scene releases its shapes before the window closes.

## Smaller sections

`Scene::drawTerrain()` calls `drawGround()`, `drawMountains()`,
`drawBuilding()`, and `drawTrees()`.

The control panel has separate functions for animation, camera,
lighting, and live coordinates.

The movement maths, colours, controls, and drawing order are unchanged.
