#include "DeferredRenderer.h"

DeferredRenderer::~DeferredRenderer()
{
	if (gBufferID != -1)
	{
		glDeleteFramebuffers(1, &gBufferID);
		// Delete it's attachments/textures too
	}
}

void DeferredRenderer::render()
{
	// Using a different set of shaders we render to a framebuffer with channels for various properties
	// Then finish the render in passes
}

void DeferredRenderer::resizeFramebuffer(int width, int height)
{
	// Delete the framebuffer if it exists and create a new one
	if (gBufferID != -1)
	{
		glDeleteFramebuffers(1, &gBufferID);
		// Delete it's attachments/textures too
	}

	glGenFramebuffers(1, &gBufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferID);
	GLuint gPosID, gNormalID, gColorSpecID;

	// Setup the position buffer
	glGenTextures(1, &gPosID);
	glBindTexture(GL_TEXTURE_2D, gPosID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosID, 0);

	// Setup the normal buffer
	glGenTextures(1, &gNormalID);
	glBindTexture(GL_TEXTURE_2D, gNormalID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormalID, 0);

	// Setup the color buffer
	glGenTextures(1, &gColorSpecID);
	glBindTexture(GL_TEXTURE_2D, gColorSpecID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gColorSpecID, 0);

	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);
}