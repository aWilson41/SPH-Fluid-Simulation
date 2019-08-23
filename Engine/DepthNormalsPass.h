#pragma once
#include "RenderPass.h"

class ShaderProgram;

class DepthNormalsPass : public RenderPass
{
public:
	DepthNormalsPass();
	~DepthNormalsPass();

public:
	void setDepthInput(GLuint* depthInput) { setInput(0, depthInput); }

	GLuint* getNormalOutput() { return &normalTexID; }

	void setMaxDepth(GLfloat maxDepth) { DepthNormalsPass::maxDepth = maxDepth; }

public:
	void render(DeferredRenderer* ren) override;
	void resizeFramebuffer(int width, int height) override;

private:
	ShaderProgram* shader = nullptr;
	GLuint normalTexID = -1;

	GLfloat maxDepth = 0.999f;
};