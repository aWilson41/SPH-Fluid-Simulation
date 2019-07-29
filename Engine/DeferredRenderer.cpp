#include "DeferredRenderer.h"
#include "AbstractMapper.h"
#include "GeometryPass.h"
#include "LightingPass.h"
#include "PlaneSource.h"
#include "PolyDataMapper.h"
#include "Shaders.h"

DeferredRenderer::DeferredRenderer(bool useDefaults)
{
	// A non-zero vao must be bound even if not using vertex attributes for quad pass
	glGenVertexArrays(1, &emptyVaoID);

	DeferredRenderer::useDefaults = useDefaults;
	if (useDefaults)
	{
		GeometryPass* geomPass = new GeometryPass();
		LightingPass* lightPass = new LightingPass();

		geomPass->setNextPass(lightPass);

		renderPasses.push_back(geomPass);
		renderPasses.push_back(lightPass);
	}
}

DeferredRenderer::~DeferredRenderer()
{
	glDeleteVertexArrays(1, &emptyVaoID);
	for (UINT i = 0; i < renderPasses.size(); i++)
	{
		delete renderPasses[i];
	}
}

void DeferredRenderer::render()
{
	if (renderPasses.size() != 0)
		renderPasses[0]->render(this);
}

void DeferredRenderer::pass()
{
	// Render the geometry
	for (UINT i = 0; i < mappers.size(); i++)
	{
		AbstractMapper* mapper = mappers[i];
		mapper->use(this);
		mapper->draw(this);
	}
}

void DeferredRenderer::quadPass()
{
	// Then render the quad
	glBindVertexArray(emptyVaoID);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

// Resizes the framebuffer (deletes and recreates), can also be used for initialization
void DeferredRenderer::resizeFramebuffer(int width, int height)
{
	Renderer::resizeFramebuffer(width, height);

	for (UINT i = 0; i < renderPasses.size(); i++)
	{
		renderPasses[i]->resizeFramebuffer(width, height);
	}
}