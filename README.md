# Wind Turbine Farm Drone Tour

A modern OpenGL scene containing six wind turbines with rotating blades, an
automatic drone route through the farm, a moving service vehicle with rotating
wheels, a road, farm buildings, terrain details, mountains, fixed sunlight and
long planar shadows. A Dear ImGui panel provides live interactive control and
displays the current scene coordinates.

## Build on macOS

```bash
xcode-select --install
brew install cmake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
./build/wind_turbine_farm
```

The first configuration downloads Dear ImGui into the local `.deps` folder.
Later builds reuse that copy.

Or open this folder in VS Code, press **Cmd+Shift+B**, then choose
**Terminal > Run Task > Run Wind Farm**.

## Controls

| Key | Action |
|---|---|
| `Space` | Pause/resume everything |
| `C` | Camera: drone route, overview orbit, vehicle follow |
| `D` | Stop/start drone movement |
| `T` | Stop/start turbine blades |
| `V` | Stop/start service vehicle |
| `H` | Shadows on/off |
| `L` | Lighting on/off |
| `A` | Coordinate axes on/off |
| Left/Right | Rotate overview camera |
| Up/Down | Raise/lower overview camera |
| `+` / `-` | Zoom overview camera |
| `R` | Reset |
| `Esc` | Exit |

The road and vehicle follow the X-axis, `+Y` is upward, and `+Z` crosses the farm.

## Control panel

The on-screen panel provides:

- Start/pause controls for the complete scene, drone, turbines and vehicle
- Drone, turbine and vehicle speed sliders
- Drone-route, overview-orbit and vehicle-follow camera selection
- Lighting and shadow checkboxes
- Editable sun X, height and Z controls
- Coordinate-axis visibility
- Reset button
- Live camera position, vehicle position, blade angle and frame rate

The original keyboard controls remain available. Keyboard shortcuts are ignored
while the control panel is actively capturing keyboard input.
