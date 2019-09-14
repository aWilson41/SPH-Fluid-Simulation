#pragma once
#include "Renderer.h"

class RenderPass;

// Implements a deferred rendering process
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

	void addPass(RenderPass* pass)
	{
		renderPasses.push_back(pass);
		PassesModified = true;
	}
	void removePass(RenderPass* pass);

	void resizeFramebuffer(int width, int height) override;

	void setColorFboID(GLuint fboID) { colorFboID = fboID; }
	void setDepthFboID(GLuint fboID) { depthFboID = fboID; }

private:
	GLuint emptyVaoID = -1;
	bool useDefaults = true;
	std::vector<RenderPass*> renderPasses;
	// The color and depth fbo's to blit after everything is rendered
	GLuint colorFboID = -1;
	GLuint depthFboID = -1;
	bool PassesModified = false;
};