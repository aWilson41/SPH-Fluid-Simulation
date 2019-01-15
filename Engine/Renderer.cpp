#include "Renderer.h"
#include "PolyDataMapper.h"
#include "Shaders.h"
#include "Material.h"

Renderer::Renderer() { Shaders::initShaders(); }

Renderer::~Renderer()
{
	Shaders::deleteShaders();

	for (UINT i = 0; i < materials.size(); i++)
	{
		delete materials[i];
	}
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
	for (UINT i = 0; i < mappers.size(); i++)
	{
		mappers[i]->draw(this);
	}
}