#pragma once
#include "MathHelper.h"

class DeferredRenderer;

class RenderPass
{
public:
	void setNextPass(RenderPass* renderPass) { nextPass = renderPass; }
	void setPassDim(int width, int height)
	{
		fboWidth = width;
		fboHeight = height;
	}

	virtual void render(DeferredRenderer* ren) = 0;

	virtual void resizeFramebuffer(int width, int height) = 0;

	void executeNextPass(DeferredRenderer* ren)
	{
		if (nextPass != nullptr)
			nextPass->render(ren);
	}

protected:
	RenderPass* nextPass = nullptr;
	int fboWidth = 100;
	int fboHeight = 100;
};