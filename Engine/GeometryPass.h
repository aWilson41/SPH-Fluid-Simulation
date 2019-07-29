#pragma once
#include "RenderPass.h"

class GeometryPass : public RenderPass
{
public:
	GeometryPass();
	~GeometryPass();

public:
	void render(DeferredRenderer* ren) override;

	void resizeFramebuffer(int width, int height) override;

private:
	GLuint gBufferID = -1;
	GLuint gPosTexID = -1;
	GLuint gNormalTexID = -1;
	GLuint gDiffuseColorTexID = -1;
	GLuint gAmbientColorTexID = -1;
	GLuint gDepthBufferID = -1;
};