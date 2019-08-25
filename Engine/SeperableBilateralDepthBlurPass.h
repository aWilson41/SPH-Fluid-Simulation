#pragma once
#include "RenderPass.h"

class ShaderProgram;

class SeperableBilateralDepthBlurPass : public RenderPass
{
public:
	SeperableBilateralDepthBlurPass();
	~SeperableBilateralDepthBlurPass();

public:
	void setDepthInput(GLuint* depthInput) { setInput(0, depthInput); }

	GLuint* getDepthOutput() { return &depthTexID; }

	// Area of effect (radius for the blur)
	void setBlurDir(glm::vec2 dir) { blurDir = dir; }
	void setRadius(GLuint radius) { blurRadius = radius; }
	void SetSigmaI(GLfloat sigma) { sigmaI = sigma; }
	void SetSigmaS(GLfloat sigma) { sigmaS = sigma; }

public:
	void render(DeferredRenderer* ren) override;
	void resizeFramebuffer(int width, int height) override;

private:
	ShaderProgram* shader = nullptr;
	GLuint depthTexID = -1;

	glm::vec2 blurDir = glm::vec2(1.0f, 0.0f);
	GLuint blurRadius = 8;
	GLfloat sigmaI = 12.0f;
	GLfloat sigmaS = 16.0f;
};