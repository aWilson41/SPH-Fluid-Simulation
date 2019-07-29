#pragma once
#include "Renderer.h"

class RenderPass;

// Implements a deferred renderings process
// Specifically, it implements render passes which manage various framebuffers
class DeferredRenderer : public Renderer
{
public:
	DeferredRenderer(bool useDefaults = true);
	~DeferredRenderer();

public:
	void render() override;

	void pass();
	void quadPass();

	void resizeFramebuffer(int width, int height) override;

	std::string getShaderDirectory() override { return "DeferredRasterize/"; };

private:
	GLuint emptyVaoID = -1;
	bool useDefaults = true;
	std::vector<RenderPass*> renderPasses;
};