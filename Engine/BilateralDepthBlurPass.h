#pragma once
#include "RenderPass.h"

class ShaderProgram;

class BilateralDepthBlurPass : public RenderPass
{
public:
	BilateralDepthBlurPass();
	~BilateralDepthBlurPass();

public:
	void setDepthInput(GLuint* depthInput) { setInput(0, depthInput); }

	GLuint* getDepthOutput() { return &depthTexID; }

	// Area of effect (radius for the blur)
	void setRadius(GLuint radius) { blurRadius = radius; }
	void SetSigmaI(GLfloat sigma) { sigmaI = sigma; }
	void SetSigmaS(GLfloat sigma) { sigmaS = sigma; }

public:
	void render(DeferredRenderer* ren) override;
	void resizeFramebuffer(int width, int height) override;

private:
	ShaderProgram* shader = nullptr;
	GLuint depthTexID = -1;

	GLuint blurRadius = 10;
	GLfloat sigmaI = 5.0f;
	GLfloat sigmaS = 5.0f;
};