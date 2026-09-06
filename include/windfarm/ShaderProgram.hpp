#pragma once

#include <OpenGL/gl3.h>

namespace windfarm {

// Compiles the vertex/fragment shaders and links them into one GPU program.
// Returns 0 when compilation or linking fails.
GLuint createShaderProgram();

} // namespace windfarm
