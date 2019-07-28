#pragma once
#include "Renderer.h"

class PlaneSource;
class PolyDataMapper;

// Implements a deferred renderings process
// Uses the custom framebuffer
class DeferredRenderer : public Renderer
{
public:
	DeferredRenderer();
	~DeferredRenderer();

public:
	void render() override;

	void resizeFramebuffer(int width, int height) override;

	std::string getShaderDirectory() override { return "DeferredRasterize/"; };

private:
	GLuint gBufferID = -1;
	GLuint gPosTexID = -1;
	GLuint gNormalTexID = -1;
	GLuint gDiffuseColorTexID = -1;
	GLuint gAmbientColorTexID = -1;
	GLuint gDepthBufferID = -1;

	ShaderProgram* lightingPassShader = nullptr;

	// Default framebuffer size, likely to get resized
	int framebufferWidth = 100;
	int framebufferHeight = 100;
};