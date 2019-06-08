#pragma once
#include "MathHelper.h"

class AbstractMapper;
class Camera;
class Material;
class RenderPass;
class ShaderProgram;

// Does the rendering, mostly just managing the scene (there is no scene object)
class Renderer
{
public:
	Renderer();
	~Renderer();

public:
	void render();

	// Might split mapper into actor where this becomes addActor
	void addRenderItem(AbstractMapper* mapper) { mappers.push_back(mapper); }
	void addMaterial(Material material);

	bool containsRenderItem(AbstractMapper* mapper);

	Material* getMaterial(UINT i) { return materials[i]; }
	AbstractMapper* getRenderItem(UINT i) { return mappers[i]; }

	// Returns the currently bound shader
	ShaderProgram* getCurrentShaderProgram() { return currShaderProgram; }
	Camera* getCamera() { return cam; }

	void setCurrentShaderProgram(ShaderProgram* program) { currShaderProgram = program; }
	void setCamera(Camera* cam) { Renderer::cam = cam; }

protected:
	// Will eventually hold actors instead of mappers
	std::vector<AbstractMapper*> mappers;
	std::vector<Material*> materials;
	Camera* cam = nullptr;
	ShaderProgram* currShaderProgram = nullptr;
	bool initialized = false;

	RenderPass* startingPass = nullptr;
	std::vector<RenderPass*> passes;
};