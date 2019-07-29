#pragma once
#include "RenderPass.h"

class ShaderProgram;

// Implements a basic phong lighting pass on the default fbo
// Should give it's own fbo eventually
class LightingPass : public RenderPass
{
public:
	LightingPass();
	~LightingPass();

public:
	void render(DeferredRenderer* ren) override;

	void resizeFramebuffer(int width, int height) override;

private:
	GLuint fboID = -1;
	GLuint colorTexID = -1;
	GLuint depthBufferID = -1;
	ShaderProgram* shader = nullptr;
};