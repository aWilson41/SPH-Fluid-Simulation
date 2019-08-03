#pragma once
#include "Renderer.h"

class RenderPass;

// Implements a deferred rendering process
// Specifically, it implements render passes which manage various framebuffers
// Right now it only supports linear passes. One of the particular challenges with
// making such a system is making sure the input buffers of a pass exist in the 
// previous pass.
class DeferredRenderer : public Renderer
{
public:
	DeferredRenderer(bool useDefaults = true);
	~DeferredRenderer();

public:
	void render() override;

	void pass();
	void quadPass();

	void addPass(RenderPass* pass) { renderPasses.push_back(pass); }

	void resizeFramebuffer(int width, int height) override;

	std::string getShaderDirectory() override { return "DeferredRasterize/"; };

	void setColorFboID(GLuint fboID) { colorFboID = fboID; }
	void setDepthFboID(GLuint fboID) { depthFboID = fboID; }

private:
	GLuint emptyVaoID = -1;
	bool useDefaults = true;
	std::vector<RenderPass*> renderPasses;
	// The color and depth fbo's to blit after everything is rendered
	GLuint colorFboID = -1;
	GLuint depthFboID = -1;
};