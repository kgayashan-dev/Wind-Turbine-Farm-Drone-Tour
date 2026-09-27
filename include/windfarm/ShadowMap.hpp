// Read this header once.
#pragma once

// OpenGL drawing.
#include <OpenGL/gl3.h>

namespace windfarm {

// Owns the depth texture that stores the scene as seen from the sun.
class ShadowMap {
public:
  bool initialize(int size = 2048); //  Create a square depth texture and framebuffer for the shadow map.
  void beginDepthPass() const; // Set the viewport to the shadow-map size and draw into the depth texture.
  void endDepthPass(int windowWidth, int windowHeight) const; // Restore the default framebuffer and viewport after drawing the depth texture.
  void bindTexture(GLenum textureUnit) const; // Bind the depth texture to a texture unit for use in the scene shader.
  void shutdown(); // Release the depth texture and framebuffer resources.

private:
  GLuint framebuffer_ = 0; // The framebuffer object that holds the depth texture.
  GLuint depthTexture_ = 0; // The depth texture that stores the scene as seen from the sun.
  int size_ = 0; // The width and height of the depth texture in pixels (square).
};

} // namespace windfarm


/*

The ShadowMap class owns the framebuffer and depth texture used for shadows. beginDepthPass 
redirects rendering from the window into the depth texture. After the scene has been  rendered from the sun,
 endDepthPass restores the normal window. bindTexture makes 
 the stored depths available to the fragment shader, and shutdown releases the GPU resources

*/