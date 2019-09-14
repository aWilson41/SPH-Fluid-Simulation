#include "DeferredRenderer.h"
#include "AbstractMapper.h"
#include "GeometryPass.h"
#include "LightingPass.h"
#include "PlaneSource.h"
#include "PolyDataMapper.h"
#include "Shaders.h"

DeferredRenderer::DeferredRenderer(bool useDefaults)
{
	shaderGroup = "DeferredRasterize";

	// A non-zero vao must be bound even if not using vertex attributes for quad pass
	glGenVertexArrays(1, &emptyVaoID);

	DeferredRenderer::useDefaults = useDefaults;
	if (useDefaults)
	{
		GeometryPass* geomPass = new GeometryPass();
		LightingPass* lightPass = new LightingPass();

		lightPass->setPosInput(geomPass->getPosOutput());
		lightPass->setNormalInput(geomPass->getNormalOutput());
		lightPass->setDiffuseInput(geomPass->getDiffuseOutput());
		lightPass->setAmbientInput(geomPass->getAmbientOutput());

		addPass(geomPass);
		addPass(lightPass);
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
	if (PassesModified)
		resizeFramebuffer(defaultFboWidth, defaultFboHeight);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Execute all the render passses
	for (UINT i = 0; i < renderPasses.size(); i++)
	{
		renderPasses[i]->render(this);
	}

	// Blit the results to the default fbo
	//printf("Blitting color from fbo %d to 0\n", colorFboID);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, colorFboID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, defaultFboWidth, defaultFboHeight, 0, 0, defaultFboWidth, defaultFboHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	//printf("Blitting depth from fbo %d to 0\n", depthFboID);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, depthFboID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, defaultFboWidth, defaultFboHeight, 0, 0, defaultFboWidth, defaultFboHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

void DeferredRenderer::removePass(RenderPass* pass)
{
	size_t passIndex = -1;
	for (size_t i = 0; i < renderPasses.size(); i++)
	{
		if (pass = renderPasses[i])
		{
			passIndex = i;
			break;
		}
	}
	if (passIndex == -1)
		return;
	renderPasses.erase(renderPasses.begin() + passIndex);
	PassesModified = true;
}

// Resizes the framebuffer (deletes and recreates), can also be used for initialization
void DeferredRenderer::resizeFramebuffer(int width, int height)
{
	Renderer::resizeFramebuffer(width, height);

	// Recursively resize framebuffers
	for (UINT i = 0; i < renderPasses.size(); i++)
	{
		renderPasses[i]->resizeFramebuffer(width, height);
	}
	PassesModified = false;
}