#pragma once
#include "RenderPass.h"

class ShaderProgram;

// Does 6 direct passes to a cube map
class CubeMapPass : public RenderPass
{
public:
	CubeMapPass();
	~CubeMapPass();

public:
	GLuint* getColorOutput() { return &cubeMapTexID; }
	GLuint* getDepthOutput() { return &cubeMapDepthTexID; }

public:
	void render(DeferredRenderer* ren) override;
	void resizeFramebuffer(int width, int height) override;

private:
	ShaderProgram* shader = nullptr;
	GLuint cubeMapTexID = -1;
	GLuint cubeMapDepthTexID = -1;
};