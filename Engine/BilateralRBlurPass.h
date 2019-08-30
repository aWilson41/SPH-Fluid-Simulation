#pragma once
#include "RenderPass.h"

class ShaderProgram;

class BilateralRBlurPass : public RenderPass
{
public:
	BilateralRBlurPass();
	~BilateralRBlurPass();

public:
	void setColorInput(GLuint* colorInput) { setInput(0, colorInput); }

	GLuint* getColorOutput() { return &colorTexID; }

	// Area of effect (radius for the blur)
	void setRadius(GLuint radius) { blurRadius = radius; }
	void SetSigmaI(GLfloat sigma) { sigmaI = sigma; }
	void SetSigmaS(GLfloat sigma) { sigmaS = sigma; }

public:
	void render(DeferredRenderer* ren) override;
	void resizeFramebuffer(int width, int height) override;

private:
	ShaderProgram* shader = nullptr;
	GLuint colorTexID = -1;

	GLuint blurRadius = 10;
	// Intensity domain, the difference on the intensity domain is more important
	GLfloat sigmaI = 12.0f;
	// Spatial domain
	GLfloat sigmaS = 16.0f;
};