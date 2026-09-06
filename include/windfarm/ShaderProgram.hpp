// Include this header only once per compilation unit to avoid duplicate definitions.
#pragma once

// OpenGL types and functions for GPU resources, rendering, and graphics state.
#include <OpenGL/gl3.h>

namespace windfarm {

// Compiles the vertex/fragment shaders and links them into one GPU program.
// Returns 0 when compilation or linking fails.
GLuint createShaderProgram();

} // namespace windfarm
