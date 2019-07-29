#include "GeometryPass.h"
#include "DeferredRenderer.h"
#include "Shaders.h"
#include <string>

GeometryPass::GeometryPass()
{
	
}

GeometryPass::~GeometryPass()
{
	if (gBufferID != -1)
	{
		glDeleteFramebuffers(1, &gBufferID);
		// Delete it's attachments/textures too
		glDeleteTextures(1, &gPosTexID);
		glDeleteTextures(1, &gNormalTexID);
		glDeleteTextures(1, &gDiffuseColorTexID);
		glDeleteTextures(1, &gAmbientColorTexID);
		glDeleteRenderbuffers(1, &gDepthBufferID);
	}
}

void GeometryPass::render(DeferredRenderer* ren)
{
	// Using a different set of shaders we render to a framebuffer with channels for various properties
	// Then finish the render in passes
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferID);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ren->pass();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Bind the textures from the fbo for the next pass
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosTexID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormalTexID);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gDiffuseColorTexID);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gAmbientColorTexID);

	executeNextPass(ren);

	// Copy the gbuffers depth buffer to the default for possible further forward rendering
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gBufferID); // Read from gbuffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Write to the default
	glBlitFramebuffer(0, 0, fboWidth, fboHeight, 0, 0, fboWidth, fboHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GeometryPass::resizeFramebuffer(int width, int height)
{
	setPassDim(width, height);

	// Setup the framebuffer
	// Delete the framebuffer if it exists and create a new one
	if (gBufferID != -1)
	{
		glDeleteFramebuffers(1, &gBufferID);
		// Delete it's attachments/textures too
		glDeleteTextures(1, &gPosTexID);
		glDeleteTextures(1, &gNormalTexID);
		glDeleteTextures(1, &gDiffuseColorTexID);
		glDeleteTextures(1, &gAmbientColorTexID);
		glDeleteRenderbuffers(1, &gDepthBufferID);
	}

	glGenFramebuffers(1, &gBufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferID);

	// Setup the position buffer
	glGenTextures(1, &gPosTexID);
	glBindTexture(GL_TEXTURE_2D, gPosTexID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosTexID, 0);

	// Setup the normal buffer
	glGenTextures(1, &gNormalTexID);
	glBindTexture(GL_TEXTURE_2D, gNormalTexID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormalTexID, 0);

	// Setup the diffuse color buffer
	glGenTextures(1, &gDiffuseColorTexID);
	glBindTexture(GL_TEXTURE_2D, gDiffuseColorTexID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gDiffuseColorTexID, 0);

	// Setup the ambient color buffer
	glGenTextures(1, &gAmbientColorTexID);
	glBindTexture(GL_TEXTURE_2D, gAmbientColorTexID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gAmbientColorTexID, 0);

	// Group these together so when we clear the color buffer it knows to clear all 4 of them
	unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, attachments);

	// Create and attach the depth buffer
	glGenRenderbuffers(1, &gDepthBufferID);
	glBindRenderbuffer(GL_RENDERBUFFER, gDepthBufferID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gDepthBufferID);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		printf("Error: Framebuffer incomplete\n");

	// Back to the default fbo
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}