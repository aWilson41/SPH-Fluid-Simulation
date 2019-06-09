#include "RenderPass.h"

RenderPass::RenderPass()
{
	// Setup an FBO
	glGenFramebuffers(1, &fboID);
	glBindFramebuffer(GL_FRAMEBUFFER, fboID);

	// Generate base/default texture
	textureID.push_back(-1);
	glGenTextures(1, &textureID[0]);
	glBindTexture(GL_TEXTURE_2D, textureID[0]);
}

void RenderPass::resizeBuffer(int width, int height)
{
	glBindTexture(GL_TEXTURE_2D, textureID[0]);
	// Allocate the texture for the fbo
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Now set the texture as the color buffer of the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID[0], 0);
}

void RenderPass::render()
{
	// Set the shader
}