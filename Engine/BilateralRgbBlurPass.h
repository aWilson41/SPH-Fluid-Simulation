#pragma once
#include "RenderPass.h"

class ShaderProgram;

class BilateralRgbBlurPass : public RenderPass
{
public:
	BilateralRgbBlurPass();
	~BilateralRgbBlurPass();

public:
	void setRgbInput(GLuint* rgbInput) { setInput(0, rgbInput); }

	GLuint* getRgbOutput() { return &rgbTexID; }

	// Area of effect (radius for the blur)
	void setRadius(GLuint radius) { blurRadius = radius; }
	void SetSigmaI(GLfloat sigma) { sigmaI = sigma; }
	void SetSigmaS(GLfloat sigma) { sigmaS = sigma; }

public:
	void render(DeferredRenderer* ren) override;
	void resizeFramebuffer(int width, int height) override;

private:
	ShaderProgram* shader = nullptr;
	GLuint rgbTexID = -1;

	GLuint blurRadius = 10;
	GLfloat sigmaI = 5.0f;
	GLfloat sigmaS = 5.0f;
};