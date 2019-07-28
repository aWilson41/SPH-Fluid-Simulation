#pragma once
#include "MathHelper.h"
#include "PropertyMap.h"
#include <string>

class AbstractMapper;
class Camera;
class PhongMaterial;
class ShaderProgram;

// Does the rendering, mostly just managing the scene (there is no scene object)
class Renderer
{
public:
	~Renderer();

public:
	virtual void render();
	// The direct renderer uses the default framebuffer
	virtual void resizeFramebuffer(int width, int height);

	// Might split mapper into actor where this becomes addActor
	void addRenderItem(AbstractMapper* mapper) { mappers.push_back(mapper); }
	void addMaterial(PhongMaterial material);

	bool containsRenderItem(AbstractMapper* mapper);

	PhongMaterial* getMaterial(UINT i) { return materials[i]; }
	AbstractMapper* getRenderItem(UINT i) { return mappers[i]; }

	// Returns the currently bound shader
	ShaderProgram* getCurrentShaderProgram() { return currShaderProgram; }
	Camera* getCamera() { return cam; }

	void setCurrentShaderProgram(ShaderProgram* program) { currShaderProgram = program; }
	void setCamera(Camera* cam) { Renderer::cam = cam; }

	// Returns the shader directory for this particular renderer
	virtual std::string getShaderDirectory() { return "DirectRasterize/"; };

protected:
	// Will eventually hold actors instead of mappers
	std::vector<AbstractMapper*> mappers;
	std::vector<PhongMaterial*> materials;
	Camera* cam = nullptr;
	ShaderProgram* currShaderProgram = nullptr;
	bool initialized = false;

	PropertyMap<32> sceneProperties;
	glm::vec3 lightDir = glm::vec3(0.0f, 1.0f, 1.0f); // Temporarily only supporting a single directional light
};