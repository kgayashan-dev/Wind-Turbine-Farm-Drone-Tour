// Create the shaders.
#include "windfarm/ShaderProgram.hpp"

// Console messages.
#include <iostream>
// Text storage.
#include <string>

namespace windfarm {
namespace {

// These constant expressions store the scene shader source code and enable
// compile-time initialization of the strings.
constexpr const char *kVertexShader = R"GLSL( //  vertex shader for the scene rendering with lighting and shadows
#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;
uniform mat3 normalMatrix;

out vec3 worldPosition;
out vec3 worldNormal;
out vec4 lightSpacePosition;

void main() {
    vec4 position = model * vec4(aPosition, 1.0);
    worldPosition = position.xyz;
    worldNormal = normalize(normalMatrix * aNormal);
    lightSpacePosition = lightSpaceMatrix * position;
    gl_Position = projection * view * position;
}
)GLSL";

constexpr const char *kFragmentShader = R"GLSL( //   fragment shader for the scene rendering with lighting and shadows
#version 330 core

in vec3 worldPosition;
in vec3 worldNormal;
in vec4 lightSpacePosition;

uniform vec3 objectColor;
uniform vec3 lightPosition;
uniform bool lightingEnabled;
uniform bool shadowsEnabled;
uniform bool emissive;
uniform sampler2D shadowMap; // The shadow map texture.

out vec4 color;

// Compare this surface with the closest depth seen from the sun.
float calculateShadow(vec3 N, vec3 L) {
    vec3 projected = lightSpacePosition.xyz / lightSpacePosition.w;
    projected = projected * 0.5 + 0.5;

    // Points outside the sun's view are not shadowed by this map.
    if (projected.z <= 0.0 || projected.z >= 1.0 ||
        projected.x <= 0.0 || projected.x >= 1.0 ||
        projected.y <= 0.0 || projected.y >= 1.0) {
        return 0.0;
    }

    // The angle-dependent bias prevents a surface shadowing itself.
    float bias = max(0.0025 * (1.0 - dot(N, L)), 0.0006);
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    float shadow = 0.0;

    // Percentage-closer filtering softens the blocky shadow-map edge.
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float closestDepth =
                texture(shadowMap, projected.xy + vec2(x, y) * texelSize).r;
            shadow += projected.z - bias > closestDepth ? 1.0 : 0.0;
        }
    }
    return shadow / 9.0;
}

void main() {
    if (emissive || !lightingEnabled) {
        color = vec4(objectColor, 1.0);
        return;
    }

    vec3 N = normalize(worldNormal); // normal appears to be in world space, so no need to transform it
    vec3 L = normalize(lightPosition - worldPosition); 
    float diffuse = max(dot(N, L), 0.0); // dot product of the normalized light direction and the normalized surface normal gives the cosine of the angle between them, which is used to calculate the diffuse lighting component

    float distanceFromLight = length(lightPosition - worldPosition);
    float attenuation =
        1.0 / (1.0 + 0.01 * distanceFromLight +
               0.0006 * distanceFromLight * distanceFromLight); // The attenuation factor is calculated based on the distance from the light source to the surface point. It reduces the intensity of the light as the distance increases, simulating the natural falloff of light in a scene.

    float shadow = shadowsEnabled ? calculateShadow(N, L) : 0.0; // if if shadows are enabled, calculate the shadow factor based on the surface normal and light direction; otherwise, set shadow to 0.0 (no shadow)
    vec3 ambientPart = 0.24 * objectColor;
    vec3 diffusePart =
        (1.0 - 0.76 * shadow) * 0.94 * diffuse * objectColor; // The diffuse lighting component is modulated by the shadow factor, which reduces the intensity of the diffuse light based on how much the surface is in shadow. The ambient and diffuse components are combined to produce the final color of the fragment.

    color = vec4(ambientPart + attenuation * diffusePart, 1.0); // The final color is a combination of ambient and diffuse lighting, modulated by the attenuation factor based on distance from the light source, and adjusted for shadows if enabled.
}
)GLSL";

// The depth shader renders the scene from the sun and writes only a z value.
//This defines the pair of shaders used for the shadow map depth pass
constexpr const char *kDepthVertexShader = R"GLSL(
#version 330 core

layout(location = 0) in vec3 aPosition;

uniform mat4 model;
uniform mat4 lightSpaceMatrix;

void main() {
    gl_Position = lightSpaceMatrix * model * vec4(aPosition, 1.0); // model view projection (MVP) matrix transforms the vertex position from model space to clip space
}
)GLSL";
constexpr const char *kDepthFragmentShader = R"GLSL(
#version 330 core

void main() {
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
// Allocate a string to hold the log and retrieve it.
  std::string log(static_cast<std::size_t>(length), '\0');
  if (isProgram) {
    glGetProgramInfoLog(object, length, nullptr, log.data());
  } else {
    glGetShaderInfoLog(object, length, nullptr, log.data());
  }
  std::cerr << log << '\n'; // 
}

// Compile a shader of the given type from the source code.
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

// Create a shader program from the given vertex and fragment shader source code.
GLuint createProgram(const char *vertexSource, const char *fragmentSource) {
  GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
  GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

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

} // namespace

// Create the shader program for the scene rendering with lighting and shadows.
GLuint createShaderProgram() {
  return createProgram(kVertexShader, kFragmentShader);
}
/// Create the shader program for the depth rendering from the sun's point of view.
GLuint createDepthShaderProgram() {
  return createProgram(kDepthVertexShader, kDepthFragmentShader); // sun
}

} // namespace windfarm
