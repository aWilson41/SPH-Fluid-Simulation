#pragma once
#include "RenderPass.h"

class ShaderProgram;

// Basically just copies over the depth into a new color buffer to render
class RenderDepthPass : public RenderPass
{
public:
	RenderDepthPass();
	~RenderDepthPass();

public:
	void setDepthInput(GLuint* depthInput) { setInput(0, depthInput); }

public:
	void render(DeferredRenderer* ren) override;
	void resizeFramebuffer(int width, int height) override;

private:
	ShaderProgram* shader = nullptr;
	GLuint colorTexID = -1;
};