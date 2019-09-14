#pragma once
#include "RenderPass.h"

class ShaderProgram;

// Implements a basic phong lighting pass given a gbuffer with
// position, normal, diffuse, ambient, and depth
class LightingPass : public RenderPass
{
public:
	LightingPass();
	~LightingPass();

public:
	GLuint* getColorOutput() { return &colorTexID; }

	void setPosInput(GLuint* posInput) { setInput(0, posInput); }
	void setNormalInput(GLuint* normalInput) { setInput(1, normalInput); }
	void setDiffuseInput(GLuint* diffuseInput) { setInput(2, diffuseInput); }
	void setAmbientInput(GLuint* ambientInput) { setInput(3, ambientInput); }

public:
	void render(DeferredRenderer* ren) override;
	void resizeFramebuffer(int width, int height) override;

private:
	ShaderProgram* shader = nullptr;
	GLuint colorTexID = -1;
	GLuint depthBufferID = -1;
};