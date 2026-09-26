// Shadow-map resource management.
#include "windfarm/ShadowMap.hpp"

// Console messages.
#include <iostream>

namespace windfarm {

bool ShadowMap::initialize(int size) {
  size_ = size;

  // A shadow map is a texture that stores only depth values.
  glGenFramebuffers(1, &framebuffer_);
  glGenTextures(1, &depthTexture_);
  glBindTexture(GL_TEXTURE_2D, depthTexture_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, size_, size_, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  const float borderColour[] = {1.0f, 1.0f, 1.0f, 1.0f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColour);

  // Attach the depth texture to a framebuffer with no colour output.
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         depthTexture_, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

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

void ShadowMap::beginDepthPass() const {
  glViewport(0, 0, size_, size_);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
  glClear(GL_DEPTH_BUFFER_BIT);
}

void ShadowMap::endDepthPass(int windowWidth, int windowHeight) const {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, windowWidth, windowHeight);
}

void ShadowMap::bindTexture(GLenum textureUnit) const {
  glActiveTexture(textureUnit);
  glBindTexture(GL_TEXTURE_2D, depthTexture_);
}

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
