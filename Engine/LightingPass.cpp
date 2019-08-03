#include "LightingPass.h"
#include "DeferredRenderer.h"
#include "Shaders.h"

LightingPass::LightingPass() : RenderPass("Lighting Pass")
{
	shader = Shaders::loadVSFSShader("Lighting_Pass",
		"Shaders/DeferredRasterize/Passes/lightPassVS.glsl",
		"Shaders/DeferredRasterize/Passes/lightPassFS.glsl");
	GLuint shaderID = shader->getProgramID();
	glUseProgram(shaderID);
	glUniform1i(glGetUniformLocation(shaderID, "gPosition"), 0);
	glUniform1i(glGetUniformLocation(shaderID, "gNormal"), 1);
	glUniform1i(glGetUniformLocation(shaderID, "gDiffuseColor"), 2);
	glUniform1i(glGetUniformLocation(shaderID, "gAmbientColor"), 3);
	glUseProgram(0);

	setNumberOfInputPorts(4);
	setNumberOfOutputPorts(1);
}

LightingPass::~LightingPass()
{
	if (fboID != -1)
	{
		glDeleteFramebuffers(1, &fboID);
		// Delete it's attachments/textures too
		glDeleteTextures(1, &colorTexID);
		glDeleteRenderbuffers(1, &depthBufferID);
	}
}

void LightingPass::render(DeferredRenderer* ren)
{
	// Use the default fbo to do the lighting pass
	glBindFramebuffer(GL_FRAMEBUFFER, fboID);
	glClear(GL_DEPTH_BUFFER_BIT);

	GLuint shaderID = shader->getProgramID();
	glUseProgram(shaderID);

	// Set the scene uniforms
	GLuint lightDirLocation = glGetUniformLocation(shaderID, "lightDir");
	if (lightDirLocation != -1)
		glUniform3fv(lightDirLocation, 1, &ren->getLightDir()[0]);

	// Bind the textures from the last pass
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, *inputs[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, *inputs[1]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, *inputs[2]);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, *inputs[3]);

	ren->quadPass();

	// Set this as the color buffer to use
	ren->setColorFboID(fboID);

	// Return to the default fbo
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void LightingPass::resizeFramebuffer(int width, int height)
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
	//printf("Created lighting pass fbo %d\n", fboID);

	// Setup the diffuse color buffer
	glGenTextures(1, &colorTexID);
	glBindTexture(GL_TEXTURE_2D, colorTexID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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