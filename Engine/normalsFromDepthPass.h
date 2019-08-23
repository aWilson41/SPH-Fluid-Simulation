#pragma once
#include "RenderPass.h"

class ShaderProgram;

class NormalsFromDepthPass : public RenderPass
{
public:
	NormalsFromDepthPass();
	~NormalsFromDepthPass();

public:
	void setDepthInput(GLuint* depthInput) { setInput(0, depthInput); }

	GLuint* getNormalOutput() { return &normalTexID; }

public:
	void render(DeferredRenderer* ren) override;
	void resizeFramebuffer(int width, int height) override;

private:
	ShaderProgram* shader = nullptr;
	GLuint normalTexID = -1;
};