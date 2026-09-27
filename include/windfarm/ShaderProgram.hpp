// Read this header once.
#pragma once

// OpenGL drawing.
#include <OpenGL/gl3.h>

namespace windfarm {

GLuint createShaderProgram();
GLuint createDepthShaderProgram(); // Create the shader program for the depth rendering from the sun's point of view.

} // namespace windfarm
