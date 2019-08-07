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
	void setRadius(GLfloat radius) { blurRadius = radius; }
	void setScale(GLfloat scale) { blurScale = scale; }
	void setFalloff(GLfloat falloff) { blurDepthFalloff = falloff; }
	void setBlurDir(glm::vec2 direction) { blurDir = direction; }

public:
	void render(DeferredRenderer* ren) override;
	void resizeFramebuffer(int width, int height) override;

private:
	ShaderProgram* shader = nullptr;
	GLuint depthTexID = -1;

	GLfloat blurRadius = 10.0f;
	GLfloat blurScale = 1.0f;
	GLfloat blurDepthFalloff = 1.0f;
	glm::vec2 blurDir = glm::vec2(0.0f, 1.0f);
};