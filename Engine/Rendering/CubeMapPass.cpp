#include "CubeMapPass.h"
#include "DeferredRenderer.h"
#include "Shaders.h"

CubeMapPass::CubeMapPass() : RenderPass("CubeMapPass")
{
	setNumberOfInputPorts(0);
	setNumberOfOutputPorts(2);
}

CubeMapPass::~CubeMapPass()
{
	if (fboID != -1)
	{
		glDeleteFramebuffers(1, &fboID);
		// Delete it's attachments/textures too
		glDeleteTextures(1, &cubeMapTexID);
		glDeleteTextures(1, &cubeMapDepthTexID);
	}
}

void CubeMapPass::render(DeferredRenderer* ren)
{
	// Direct the renderer to use direct rendering for this pass
	std::string shaderGroup = ren->getShaderGroup();
	ren->setShaderGroup("DirectRasterize");

	// Using a different set of shaders we render to a framebuffer with channels for various properties
	// Then finish the render in passes
	glBindFramebuffer(GL_FRAMEBUFFER, fboID);

	ren->pass();

	// Return to the default fbo
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	ren->setShaderGroup(shaderGroup);
}

void CubeMapPass::resizeFramebuffer(int width, int height)
{
	setPassDim(width, height);

	// Setup the framebuffer
	// Delete the framebuffer if it exists and create a new one
	if (fboID != -1)
	{
		glDeleteFramebuffers(1, &fboID);
		// Delete it's attachments/textures too
		glDeleteTextures(1, &cubeMapTexID);
		glDeleteTextures(1, &cubeMapDepthTexID);
	}

	glGenFramebuffers(1, &fboID);
	glBindFramebuffer(GL_FRAMEBUFFER, fboID);


	// Setup the color buffer
	glGenTextures(1, &cubeMapTexID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexID);
	for (GLuint i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, fboWidth, fboHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP, cubeMapTexID, 0);
	unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);


	// Setup the depth buffer
	glGenRenderbuffers(1, &cubeMapDepthTexID);
	glBindRenderbuffer(GL_RENDERBUFFER, cubeMapDepthTexID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fboWidth, fboHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fboID);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		printf("Error: Framebuffer incomplete\n");

	// Back to the default fbo
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	outputs[0] = cubeMapTexID;
	outputs[1] = cubeMapDepthTexID;
}