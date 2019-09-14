#pragma once
#include "RenderPass.h"

class GeometryPass : public RenderPass
{
public:
	GeometryPass();
	~GeometryPass();

public:
	GLuint* getPosOutput() { return &gPosTexID; }
	GLuint* getNormalOutput() { return &gNormalTexID; }
	GLuint* getDiffuseOutput() { return &gDiffuseColorTexID; }
	GLuint* getAmbientOutput() { return &gAmbientColorTexID; }
	GLuint* getDepthOutput() { return &gDepthTexID; }

public:
	void render(DeferredRenderer* ren) override;
	void resizeFramebuffer(int width, int height) override;

private:
	GLuint gPosTexID = -1;
	GLuint gNormalTexID = -1;
	GLuint gDiffuseColorTexID = -1;
	GLuint gAmbientColorTexID = -1;
	GLuint gDepthTexID = -1;
};