#include "GeometryPass.h"
#include "DeferredRenderer.h"
#include "Shaders.h"

GeometryPass::GeometryPass() : RenderPass("Geometry Pass")
{
	setNumberOfInputPorts(0);
	setNumberOfOutputPorts(5);
}

GeometryPass::~GeometryPass()
{
	if (fboID != -1)
	{
		glDeleteFramebuffers(1, &fboID);
		// Delete it's attachments/textures too
		glDeleteTextures(1, &gPosTexID);
		glDeleteTextures(1, &gNormalTexID);
		glDeleteTextures(1, &gDiffuseColorTexID);
		glDeleteTextures(1, &gAmbientColorTexID);
		glDeleteTextures(1, &gDepthTexID);
	}
}

void GeometryPass::render(DeferredRenderer* ren)
{
	// Using a different set of shaders we render to a framebuffer with channels for various properties
	// Then finish the render in passes
	glBindFramebuffer(GL_FRAMEBUFFER, fboID);

	glClearTexImage(gPosTexID, 0, GL_RGB, GL_FLOAT, 0);
	glClearTexImage(gNormalTexID, 0, GL_RGB, GL_FLOAT, 0);
	static GLubyte diffuseClearColor[4] = { 0, 0, 0, 255 };
	glClearTexImage(gDiffuseColorTexID, 0, GL_RGBA, GL_UNSIGNED_BYTE, diffuseClearColor);
	float* clearColor = ren->getClearColor();
	static GLubyte ambientClearColor[4] = {
		static_cast<GLubyte>(clearColor[0] * 255.0f), 
		static_cast<GLubyte>(clearColor[1] * 255.0f),
		static_cast<GLubyte>(clearColor[2] * 255.0f),
		static_cast<GLubyte>(clearColor[3] * 255.0f) };
	glClearTexImage(gAmbientColorTexID, 0, GL_RGBA, GL_UNSIGNED_BYTE, ambientClearColor);
	static float depthClearValue = 1.0f;
	glClearTexImage(gDepthTexID, 0, GL_DEPTH_COMPONENT, GL_FLOAT, &depthClearValue);

	ren->pass();

	ren->setDepthFboID(fboID);

	// Return to the default fbo
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GeometryPass::resizeFramebuffer(int width, int height)
{
	setPassDim(width, height);

	// Setup the framebuffer
	// Delete the framebuffer if it exists and create a new one
	if (fboID != -1)
	{
		glDeleteFramebuffers(1, &fboID);
		// Delete it's attachments/textures too
		glDeleteTextures(1, &gPosTexID);
		glDeleteTextures(1, &gNormalTexID);
		glDeleteTextures(1, &gDiffuseColorTexID);
		glDeleteTextures(1, &gAmbientColorTexID);
		glDeleteTextures(1, &gDepthTexID);
	}

	glGenFramebuffers(1, &fboID);
	glBindFramebuffer(GL_FRAMEBUFFER, fboID);

	// Setup the position buffer
	glGenTextures(1, &gPosTexID);
	glBindTexture(GL_TEXTURE_2D, gPosTexID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosTexID, 0);

	// Setup the normal buffer
	glGenTextures(1, &gNormalTexID);
	glBindTexture(GL_TEXTURE_2D, gNormalTexID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormalTexID, 0);

	// Setup the diffuse color buffer
	glGenTextures(1, &gDiffuseColorTexID);
	glBindTexture(GL_TEXTURE_2D, gDiffuseColorTexID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gDiffuseColorTexID, 0);

	// Setup the ambient color buffer
	glGenTextures(1, &gAmbientColorTexID);
	glBindTexture(GL_TEXTURE_2D, gAmbientColorTexID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gAmbientColorTexID, 0);

	// Group these together so when we clear the color buffer it knows to clear all 4 of them
	unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, attachments);

	// Create and attach the depth buffer
	glGenTextures(1, &gDepthTexID);
	glBindTexture(GL_TEXTURE_2D, gDepthTexID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepthTexID, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		printf("Error: Framebuffer incomplete\n");

	// Back to the default fbo
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	outputs[0] = gPosTexID;
	outputs[1] = gNormalTexID;
	outputs[2] = gDiffuseColorTexID;
	outputs[3] = gAmbientColorTexID;
	outputs[4] = gDepthTexID;
}