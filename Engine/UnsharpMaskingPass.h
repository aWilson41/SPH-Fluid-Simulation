#pragma once
#include "RenderPass.h"

class ShaderProgram;

// This pass provides a cheap AO trick.
// From "Image Enhancement by Unsharp Masking the Depth Buffer; Luft, et al. Siggraph 2006
// We essentially blur the depth buffer and subtract it from the original depth buffer
// Then blend into the color.
class UnsharpMaskingPass : public RenderPass
{
public:
	UnsharpMaskingPass();
	~UnsharpMaskingPass();

public:
	void setColorInput(GLuint* colorInput) { setInput(0, colorInput); }
	void setDepthInput(GLuint* depthInput) { setInput(1, depthInput); }

	// Enhances the effect
	void setDarknessFactor(GLfloat darknessFactor) { UnsharpMaskingPass::darknessFactor = darknessFactor; }
	// Area of effect (radius for the gaussian blur)
	void setRadius(GLuint radius) { UnsharpMaskingPass::radius = radius; }
	// If specified will use a ratio of the buffer diagonal instead
	void setRadiusRatio(GLfloat radiusRatio) { UnsharpMaskingPass::radiusRatio = radiusRatio; }
	// Gaussian blur sigma, if not specified will use sigma = blurRadius
	void setSigma(GLfloat sigma) { UnsharpMaskingPass::sigma = sigma; }

public:
	void render(DeferredRenderer* ren) override;
	void resizeFramebuffer(int width, int height) override;

private:
	ShaderProgram* shader = nullptr;
	GLuint colorTexID = -1;
	GLuint depthBufferID = -1;

	GLfloat darknessFactor = 5.0f;
	GLuint radius = 20;
	GLfloat radiusRatio = -1.0f;
	GLfloat sigma = 2.0f;
};