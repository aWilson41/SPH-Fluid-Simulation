#pragma once
#include "RenderPass.h"

class ShaderProgram;

class LightingPass : public RenderPass
{
public:
	LightingPass();
	~LightingPass();

public:
	void render(DeferredRenderer* ren) override;

	void resizeFramebuffer(int width, int height) override;

private:
	ShaderProgram* lightingPassShader = nullptr;
};