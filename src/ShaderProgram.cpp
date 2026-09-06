// Create the shaders.
#include "windfarm/ShaderProgram.hpp"

// Console messages.
#include <iostream>
// Text storage.
#include <string>

namespace windfarm {
namespace {

// Move points onto the screen.
constexpr const char *kVertexShader = R"GLSL(
#version 330 core

# Vertex shader: moves each vertex to its position on the screen.
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

// Matrices to move points from model space to world space, then view space, then clip space.
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;//
uniform vec3 lightPosition; // The position of the light source in world space. The light source is the sun, which is a directional light source, so its position is very far away.
uniform float shadowPlaneY; // The Y coordinate of the plane on which to draw shadows. The shadows are drawn on the ground, so this is the ground's Y coordinate.

// Whether to render shadows.
uniform bool shadowMode; 

out vec3 worldPosition;
out vec3 normal;


void main() {
    vec4 position = model * vec4(aPosition, 1.0);

    if (shadowMode) {
        float t = (shadowPlaneY - lightPosition.y) /
                  (position.y - lightPosition.y);
        position.xyz = lightPosition + t * (position.xyz - lightPosition);
        position.y += 0.018;
    }

    worldPosition = position.xyz;
    normal = normalize(normalMatrix * aNormal);

    // Model -> World -> View -> Projection (clip space).
    gl_Position = projection * view * position;
}
)GLSL";

// Fragment shader: calculates each visible pixel's colour.
// Choose each pixel colour.
constexpr const char *kFragmentShader = R"GLSL( // Fragment shader: calculates each visible pixel's colour.
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

    if (emissive || !lightingEnabled) {
        color = vec4(objectColor, 1.0);
        return;
    }

    // N, L, V and H are the normal, light, view and halfway vectors.
    vec3 N = normalize(normal);
    vec3 L = normalize(lightPosition - worldPosition);
    vec3 V = normalize(cameraPosition - worldPosition);
    vec3 H = normalize(L + V);

    float diffuse = max(dot(N, L), 0.0);
    float specular = pow(max(dot(N, H), 0.0), 44.0);
    float distanceFromLight = length(lightPosition - worldPosition);
    float attenuation = 1.0 /
        (1.0 + 0.01 * distanceFromLight +
         0.0006 * distanceFromLight * distanceFromLight);

    vec3 ambientPart = 0.24 * objectColor;
    vec3 diffusePart = 0.94 * diffuse * objectColor;
    vec3 specularPart = vec3(0.52) * specular;
    color = vec4(ambientPart + attenuation * (diffusePart + specularPart),
                 1.0);
}
)GLSL";

// Read and print the driver's compilation or linking diagnostics.
// Show shader errors.
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

// Prepare one shader.
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

// Join the two shaders.
GLuint createShaderProgram() {
  GLuint vertexShader = compileShader(GL_VERTEX_SHADER, kVertexShader); // Create the vertex shader.
  GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, kFragmentShader); // Create the fragment shader.

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

  // Link the shaders into a program.
  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);

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
