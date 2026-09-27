// Shadow-map resource management.
#include "windfarm/ShadowMap.hpp"

// Console messages.
#include <iostream>

namespace windfarm {

bool ShadowMap::initialize(int size) { // Create a square depth texture and framebuffer for the shadow map.
  size_ = size; // 

  // A shadow map is a texture that stores only depth values.
  glGenFramebuffers(1, &framebuffer_); // generate a framebuffer object to hold the depth texture.
  glGenTextures(1, &depthTexture_); // generate a texture object to store the depth values.
  glBindTexture(GL_TEXTURE_2D, depthTexture_); 
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, size_, size_, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); // Set the border colour to white so that fragments outside the shadow map are lit.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER); // Set the border colour to white so that fragments outside the shadow map are lit.
  const float borderColour[] = {1.0f, 1.0f, 1.0f, 1.0f}; // Set the border colour to white.
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColour); 

  // Attach the depth texture to a framebuffer with no colour output.
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         depthTexture_, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  // Check that the framebuffer is complete and ready for rendering.
  const bool complete =
      glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  if (!complete) {
    std::cerr << "Shadow-map framebuffer creation failed.\n";
    shutdown();
    return false;
  }
  return true;
}

// Set the viewport to the shadow-map size and draw into the depth texture.
//A depth texture is a texture that stores distance/depth information per pixel instead of color information.
void ShadowMap::beginDepthPass() const {
  glViewport(0, 0, size_, size_); // Set the viewport to the shadow-map size.
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
  glClear(GL_DEPTH_BUFFER_BIT);
} // Clear the depth buffer to prepare for rendering the scene from the light's perspective. This ensures that the depth values in the shadow map are accurate and not influenced by previous frames or other rendering operations.

// Restore the default framebuffer and viewport after drawing the depth texture.
void ShadowMap::endDepthPass(int windowWidth, int windowHeight) const {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, windowWidth, windowHeight);
}

void ShadowMap::bindTexture(GLenum textureUnit) const {
  glActiveTexture(textureUnit);
  glBindTexture(GL_TEXTURE_2D, depthTexture_);
}

// Release the depth texture and framebuffer resources.
void ShadowMap::shutdown() {
  if (depthTexture_ != 0) {
    glDeleteTextures(1, &depthTexture_);
    depthTexture_ = 0;
  }
  if (framebuffer_ != 0) {
    glDeleteFramebuffers(1, &framebuffer_);
    framebuffer_ = 0;
  }
  size_ = 0;
}

} // namespace windfarm
