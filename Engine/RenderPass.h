#pragma once
#include "MathHelper.h"

class RenderPass
{
public:
	// Constructs a default FBO
	RenderPass();

	// Link passes together by next pass not previous
	void setNextPass(RenderPass* pass)
	{
		nextPass = pass;
		pass->setPrevPass(this);
	}
	void setPrevPass(RenderPass* pass) { prevPass = pass; }

	virtual void resizeBuffer(int width, int height);

	virtual void render();

protected:
	GLuint fboID = -1;
	std::vector<GLuint> textureID;
	RenderPass* nextPass = nullptr;
	RenderPass* prevPass = nullptr;
};