#include "LightingPass.h"
#include "DeferredRenderer.h"
#include "Shaders.h"
#include <string>

LightingPass::LightingPass()
{
	lightingPassShader = Shaders::loadVSFSShader("Lighting_Pass", "Shaders/DeferredRasterize/LightingPass/lightPassVS.glsl", "Shaders/DeferredRasterize/LightingPass/lightPassFS.glsl");
	GLuint lightPassShaderID = lightingPassShader->getProgramID();
	glUseProgram(lightPassShaderID);
	glUniform1i(glGetUniformLocation(lightPassShaderID, "gPosition"), 0);
	glUniform1i(glGetUniformLocation(lightPassShaderID, "gNormal"), 1);
	glUniform1i(glGetUniformLocation(lightPassShaderID, "gDiffuseColor"), 2);
	glUniform1i(glGetUniformLocation(lightPassShaderID, "gAmbientColor"), 3);
	glUseProgram(0);
}

LightingPass::~LightingPass()
{

}

void LightingPass::render(DeferredRenderer* ren)
{
	// Use the default fbo to do the lighting pass
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLuint lightPassShaderID = lightingPassShader->getProgramID();
	glUseProgram(lightPassShaderID);

	// tmp

	// Set the scene uniforms
	GLuint lightDirLocation = glGetUniformLocation(lightPassShaderID, "lightDir");
	if (lightDirLocation != -1)
		glUniform3fv(lightDirLocation, 1, &ren->getLightDir()[0]);

	ren->quadPass();

	executeNextPass(ren);
}

void LightingPass::resizeFramebuffer(int width, int height)
{
	// Don't do any work since we use the default fbo
	setPassDim(width, height);
}