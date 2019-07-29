//#pragma once
//#include "RenderPass.h"
//
//class ShaderProgram;
//
//// This pass provides a cheap AO trick.
//// From "Image Enhancement by Unsharp Masking the Depth Buffer; Luft, et al. Siggraph 2006
//// We essentially blur the depth buffer and subtract it from the original depth buffer
//// Then blend into the color
//class UnsharpMaskingPass : public RenderPass
//{
//public:
//	UnsharpMaskingPass();
//	~UnsharpMaskingPass();
//
//public:
//	void render(DeferredRenderer* ren) override;
//
//	void resizeFramebuffer(int width, int height) override;
//
//private:
//	GLuint fboID = -1;
//	GLuint colorTexID = -1;
//	GLuint depthBufferID = -1;
//	ShaderProgram* shader = nullptr;
//};