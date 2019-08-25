#include "UnsharpMaskingPass.h"
#include "Camera.h"
#include "DeferredRenderer.h"
#include "Shaders.h"
#include <string>

UnsharpMaskingPass::UnsharpMaskingPass() : RenderPass("Unsharp Masking Pass")
{
	shader = Shaders::loadVSFSShader("Unsharp_Masking_Pass",
		"Shaders/DeferredRasterize/Passes/quadVS.glsl",
		"Shaders/DeferredRasterize/Passes/unsharpMaskingPassFS.glsl");
	GLuint shaderID = shader->getProgramID();
	glUseProgram(shaderID);
	glUniform1i(glGetUniformLocation(shaderID, "gColor"), 0);
	glUniform1i(glGetUniformLocation(shaderID, "gDepth"), 1);
	glUseProgram(0);

	// Takes color and depth input and outputs color and depth
	setNumberOfInputPorts(2);
	setNumberOfOutputPorts(1);
}

UnsharpMaskingPass::~UnsharpMaskingPass()
{
	if (fboID != -1)
	{
		glDeleteFramebuffers(1, &fboID);
		// Delete it's attachments/textures too
		glDeleteTextures(1, &colorTexID);
		glDeleteRenderbuffers(1, &depthBufferID);
	}
}

void UnsharpMaskingPass::render(DeferredRenderer* ren)
{
	// Use the default fbo to do the lighting pass
	glBindFramebuffer(GL_FRAMEBUFFER, fboID);
	glClear(GL_DEPTH_BUFFER_BIT);

	GLuint shaderID = shader->getProgramID();
	glUseProgram(shaderID);

	// Set some uniforms
	GLuint blurRadiusLocation = glGetUniformLocation(shaderID, "blurRadius");
	if (blurRadiusLocation != -1) // 3% of the diagonal (paper cites 2%-5%)
	{
		if (radiusRatio != -1.0f)
			radius = static_cast<GLuint>(sqrt(fboWidth * fboHeight + fboWidth * fboHeight) * radiusRatio);
		glUniform1i(blurRadiusLocation, radius);
	}
	GLuint darknessFactorLocation = glGetUniformLocation(shaderID, "darknessFactor");
	if (darknessFactorLocation != -1)
		glUniform1f(darknessFactorLocation, darknessFactor);
	GLuint sigmaLocation = glGetUniformLocation(shaderID, "sigma");
	if (sigmaLocation != -1)
	{
		if (sigma == -1.0f)
			glUniform1f(sigmaLocation, static_cast<GLfloat>(radius));
		else
			glUniform1f(sigmaLocation, sigma);
	}

	// Bind the color and depth buffer
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, *inputs[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, *inputs[1]);

	ren->quadPass();

	// Set this as the color buffer to use
	ren->setColorFboID(fboID);

	// Return to the default fbo
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void UnsharpMaskingPass::resizeFramebuffer(int width, int height)
{
	setPassDim(width, height);

	// Setup the framebuffer
	// Delete the framebuffer if it exists and create a new one
	if (fboID != -1)
	{
		glDeleteFramebuffers(1, &fboID);
		// Delete it's attachments/textures too
		glDeleteTextures(1, &colorTexID);
		glDeleteRenderbuffers(1, &depthBufferID);
	}

	glGenFramebuffers(1, &fboID);
	glBindFramebuffer(GL_FRAMEBUFFER, fboID);

	// Setup the color buffer
	glGenTextures(1, &colorTexID);
	glBindTexture(GL_TEXTURE_2D, colorTexID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexID, 0);

	// Group these together so when we clear the color buffer it knows to clear all 4 of them
	unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	// Create and attach the depth buffer
	glGenRenderbuffers(1, &depthBufferID);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferID);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		printf("Error: Framebuffer incomplete\n");

	// Back to the default fbo
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	outputs[0] = colorTexID;
}