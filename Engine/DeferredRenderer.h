#pragma once
#include "Renderer.h"

// Implements a deferred renderings process
// Uses the custom framebuffer
class DeferredRenderer : public Renderer
{
public:
	~DeferredRenderer();

public:
	void render() override;

	void resizeFramebuffer(int width, int height) override;

	std::string getShaderDirectory() override { return "DeferredRasterize/"; };

private:
	GLuint gBufferID = -1;
	// Default framebuffer size, likely to get resized
	int framebufferWidth = 100;
	int framebufferHeight = 100;
};