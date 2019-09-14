#pragma once
#include "MathHelper.h"
#include "PropertyMap.h"
#include <string>

class Renderer;

// Contains property maps that form a bitset combined of render, scene, and object properties
class ShaderProperties
{
public:
	// Updates the property combining all the properties to produce a unique 64bit value
	unsigned long long update();

	PropertyMap<32>* getObjectProperties() { return &objectProperties; }
	PropertyMap<16>* getSceneProperties() { return &sceneProperties; }
	PropertyMap<16>* getRenderProperties() { return &renderProperties; }
	unsigned long long getKey() { return keyLong; }

	void setObjectProperties(PropertyMap<32> objectProperties) { ShaderProperties::objectProperties = objectProperties; }
	void setSceneProperties(PropertyMap<16> sceneProperties) { ShaderProperties::sceneProperties = sceneProperties; }
	void setRenderProperties(PropertyMap<16> renderProperties) { ShaderProperties::renderProperties = renderProperties; }

	std::string getFullBitString() { return key.to_string(); }

	std::string getFullPropertyString();

protected:
	PropertyMap<32> objectProperties;
	PropertyMap<16> sceneProperties;
	PropertyMap<16> renderProperties;

	std::bitset<64> key;
	unsigned long long keyLong = 0;
};

class AbstractMapper
{
public:
	AbstractMapper() { objectProperties = properties.getObjectProperties(); }

	virtual GLuint getShaderProgramID() = 0;
	bool getUseCustomShader() { return useCustomShader; }
	virtual std::string getMapperName() = 0;

	void setUseCustomShader(bool useCustomShader) { AbstractMapper::useCustomShader = useCustomShader; }

	virtual void update() = 0;

	// Binds the shader program
	void use(Renderer* ren);
	virtual void useShader(std::string shaderGroup) = 0;

	virtual void draw(Renderer* ren) = 0;

protected:
	PropertyMap<32>* objectProperties = nullptr;
	ShaderProperties properties;
	bool useCustomShader = false;
};