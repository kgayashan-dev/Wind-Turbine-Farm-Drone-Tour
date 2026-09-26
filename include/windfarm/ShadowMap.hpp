// Read this header once.
#pragma once

// OpenGL drawing.
#include <OpenGL/gl3.h>

namespace windfarm {

// Owns the depth texture that stores the scene as seen from the sun.
class ShadowMap {
public:
  bool initialize(int size = 2048);
  void beginDepthPass() const;
  void endDepthPass(int windowWidth, int windowHeight) const;
  void bindTexture(GLenum textureUnit) const;
  void shutdown();

private:
  GLuint framebuffer_ = 0;
  GLuint depthTexture_ = 0;
  int size_ = 0;
};

} // namespace windfarm
