// Project shader declaration; implemented in src/ShaderProgram.cpp (GPU program creation).
#include "windfarm/ShaderProgram.hpp"

// Standard console output streams for instructions and error messages.
#include <iostream>
// String storage for shader compiler and linker diagnostic messages.
#include <string>

namespace windfarm {
namespace {

// Vertex shader: transforms vertices and optionally projects them onto the
// ground to create the planar-shadow pass.
constexpr const char *kVertexShader = R"GLSL(
#version 330 core

// Vertex attributes match the position/normal layout configured in Graphics.cpp.
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

// Uniforms are supplied by CPU draw code and shared by vertices in each draw call.
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;
uniform vec3 lightPosition;
uniform float shadowPlaneY;
uniform bool shadowMode;

// Pass world-space position and normal to the fragment shader through interpolation.
out vec3 worldPosition;
out vec3 normal;

void main() {
    // Transform this vertex from its local object space into world space.
    vec4 position = model * vec4(aPosition, 1.0);

    if (shadowMode) {
        // Intersect the line from the sun through this vertex with the ground.
        float t = (shadowPlaneY - lightPosition.y) /
                  (position.y - lightPosition.y);
        position.xyz = lightPosition + t * (position.xyz - lightPosition);
        // Lift the projected shadow slightly to reduce depth fighting with the ground.
        position.y += 0.018;
    }

    worldPosition = position.xyz;
    normal = normalize(normalMatrix * aNormal);

    // Model -> World -> View -> Projection (clip space).
    gl_Position = projection * view * position;
}
)GLSL";

// Fragment shader: calculates each visible pixel's colour.
constexpr const char *kFragmentShader = R"GLSL(
#version 330 core

in vec3 worldPosition;
in vec3 normal;

uniform vec3 objectColor;
uniform vec3 lightPosition;
uniform vec3 cameraPosition;
uniform bool lightingEnabled;
uniform bool shadowMode;
uniform bool emissive;

out vec4 color;

void main() {
    if (shadowMode) {
        color = vec4(0.022, 0.025, 0.030, 0.48);
        return;
    }

    // Emissive objects and the unlit mode use the original material colour directly.
    if (emissive || !lightingEnabled) {
        color = vec4(objectColor, 1.0);
        return;
    }

    // N, L, V and H are the normal, light, view and halfway vectors.
    vec3 N = normalize(normal);
    vec3 L = normalize(lightPosition - worldPosition);
    vec3 V = normalize(cameraPosition - worldPosition);
    vec3 H = normalize(L + V);

    // Diffuse brightness depends on facing the light; Blinn-Phong specular adds a tight highlight.
    float diffuse = max(dot(N, L), 0.0);
    float specular = pow(max(dot(N, H), 0.0), 44.0);
    float distanceFromLight = length(lightPosition - worldPosition);
    // Reduce light contribution as the distance from the sun position increases.
    float attenuation = 1.0 /
        (1.0 + 0.01 * distanceFromLight +
         0.0006 * distanceFromLight * distanceFromLight);

    // Combine constant ambient light with distance-attenuated diffuse and specular terms.
    vec3 ambientPart = 0.24 * objectColor;
    vec3 diffusePart = 0.94 * diffuse * objectColor;
    vec3 specularPart = vec3(0.52) * specular;
    color = vec4(ambientPart + attenuation * (diffusePart + specularPart),
                 1.0);
}
)GLSL";

// Read and print the driver's compilation or linking diagnostics.
void printShaderLog(GLuint object, bool isProgram) {
  GLint length = 0;
  if (isProgram) {
    glGetProgramiv(object, GL_INFO_LOG_LENGTH, &length);
  } else {
    glGetShaderiv(object, GL_INFO_LOG_LENGTH, &length);
  }

  if (length <= 1) {
    return;
  }

  std::string log(static_cast<std::size_t>(length), '\0');
  if (isProgram) {
    glGetProgramInfoLog(object, length, nullptr, log.data());
  } else {
    glGetShaderInfoLog(object, length, nullptr, log.data());
  }
  std::cerr << log << '\n';
}

// Compile one GLSL stage; report errors and return zero if the driver rejects it.
GLuint compileShader(GLenum type, const char *source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  GLint succeeded = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &succeeded);
  if (succeeded == GL_FALSE) {
    printShaderLog(shader, false);
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

} // namespace

// Compile both shader stages, link them, and return a program handle for drawing.
GLuint createShaderProgram() {
  GLuint vertexShader = compileShader(GL_VERTEX_SHADER, kVertexShader);
  GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, kFragmentShader);

  // Clean up whichever stage succeeded if the other failed.
  if (vertexShader == 0 || fragmentShader == 0) {
    if (vertexShader != 0) {
      glDeleteShader(vertexShader);
    }
    if (fragmentShader != 0) {
      glDeleteShader(fragmentShader);
    }
    return 0;
  }

  // Link vertex and fragment stages into one executable GPU pipeline.
  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);

  // The linked program owns the compiled code, so these objects are finished.
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  GLint succeeded = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &succeeded);
  if (succeeded == GL_FALSE) {
    printShaderLog(program, true);
    glDeleteProgram(program);
    return 0;
  }

  return program;
}

} // namespace windfarm
