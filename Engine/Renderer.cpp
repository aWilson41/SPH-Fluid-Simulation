#include "Renderer.h"
#include "Material.h"
#include "PolyDataMapper.h"
#include "RenderPass.h"
#include "Shaders.h"

Renderer::Renderer()
{
	Shaders::initShaders();

	// Setup the default pass
	//startingPass = new RenderPass();
}

Renderer::~Renderer()
{
	Shaders::deleteShaders();

	for (UINT i = 0; i < materials.size(); i++)
	{
		delete materials[i];
	}
	delete startingPass;
}

void Renderer::addMaterial(Material material) { materials.push_back(new Material(material)); }

bool Renderer::containsRenderItem(AbstractMapper* mapper)
{
	for (UINT i = 0; i < mappers.size(); i++)
	{
		if (mappers[i] == mapper)
			return true;
	}
	return false;
}

void Renderer::render()
{
	// Call the starting pass, this should recurse through all other passes
	//startingPass->render();

	// Then composite the passes

	// Finally copy the final image to the back buffer

	// Temp
	for (UINT i = 0; i < mappers.size(); i++)
	{
		mappers[i]->draw(this);
	}
}