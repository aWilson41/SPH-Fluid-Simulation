#include "Renderer.h"
#include "AbstractMapper.h"
#include "PhongMaterial.h"
#include "Shaders.h"

Renderer::~Renderer()
{
	Shaders::deleteShaders();

	for (UINT i = 0; i < materials.size(); i++)
	{
		delete materials[i];
	}
}

void Renderer::addMaterial(PhongMaterial material) { materials.push_back(new PhongMaterial(material)); }

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
		AbstractMapper* mapper = mappers[i];
		mapper->use(this);

		// Set the scene uniforms
		GLuint lightDirLocation = glGetUniformLocation(mapper->getShaderProgramID(), "lightDir");
		if (lightDirLocation != -1)
			glUniform3fv(lightDirLocation, 1, &lightDir[0]);

		mapper->draw(this);
	}
}