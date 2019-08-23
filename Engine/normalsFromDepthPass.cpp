#include "NormalsFromDepthPass.h"
#include "Camera.h"
#include "DeferredRenderer.h"
#include "Shaders.h"
#include <string>

NormalsFromDepthPass::NormalsFromDepthPass() : RenderPass("Normals From Depth Pass")
{
	shader = Shaders::loadVSFSShader("Bilateral_Rgb_Blur_Pass",
		"Shaders/DeferredRasterize/Passes/quadVS.glsl",
		"Shaders/DeferredRasterize/Passes/normalsFromDepthPass.glsl");
	GLuint shaderID = shader->getProgramID();
	glUseProgram(shaderID);
	glUniform1i(glGetUniformLocation(shaderID, "depthTex"), 0);
	glUseProgram(0);

	// Takes color and depth input and outputs color and depth
	setNumberOfInputPorts(1);
	setNumberOfOutputPorts(1);
}

NormalsFromDepthPass::~NormalsFromDepthPass()
{
	if (fboID != -1)
	{
		glDeleteFramebuffers(1, &fboID);
		// Delete it's attachments/textures too
		glDeleteTextures(1, &normalTexID);
	}
}

void NormalsFromDepthPass::render(DeferredRenderer* ren)
{
	// Use the default fbo to do the lighting pass
	glBindFramebuffer(GL_FRAMEBUFFER, fboID);
	glClear(GL_COLOR_BUFFER_BIT);

	GLuint shaderID = shader->getProgramID();
	glUseProgram(shaderID);

	// Bind the color and depth buffer
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, *inputs[0]);

	ren->quadPass();

	// Set this as the depth buffer to use
	ren->setColorFboID(fboID);

	// Return to the default fbo
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void NormalsFromDepthPass::resizeFramebuffer(int width, int height)
{
	setPassDim(width, height);

	// Setup the framebuffer
	// Delete the framebuffer if it exists and create a new one
	if (fboID != -1)
	{
		glDeleteFramebuffers(1, &fboID);
		// Delete it's attachments/textures too
		glDeleteTextures(1, &normalTexID);
	}

	glGenFramebuffers(1, &fboID);
	glBindFramebuffer(GL_FRAMEBUFFER, fboID);

	// Setup the color buffer
	glGenTextures(1, &normalTexID);
	glBindTexture(GL_TEXTURE_2D, normalTexID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, normalTexID, 0);

	// Group these together so when we clear the color buffer it knows to clear all 4 of them
	unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		printf("Error: Framebuffer incomplete\n");

	// Back to the default fbo
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	outputs[0] = normalTexID;
}