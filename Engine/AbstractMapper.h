#pragma once
#include "MathHelper.h"
#include "PropertyMap.h"

class Renderer;

class ShaderProperties
{
public:
	// Updates the property combining all the properties to produce a unique 64bit value
	unsigned long long update()
	{
		// To quickly compile these bitset we add them
		key = std::bitset<64>(
			objectProperties.getPropertyBits().to_string() + 
			sceneProperties.getPropertyBits().to_string() + 
			renderProperties.getPropertyBits().to_string());
		keyLong = key.to_ullong();
		return keyLong;
	}

	PropertyMap<32>* getObjectProperties() { return &objectProperties; }
	PropertyMap<16>* getSceneProperties() { return &sceneProperties; }
	PropertyMap<16>* getRenderProperties() { return &renderProperties; }
	unsigned long long getKey() { return keyLong; }

	void setObjectProperties(PropertyMap<32> objectProperties) { ShaderProperties::objectProperties = objectProperties; }
	void setSceneProperties(PropertyMap<16> sceneProperties) { ShaderProperties::sceneProperties = sceneProperties; }
	void setRenderProperties(PropertyMap<16> renderProperties) { ShaderProperties::renderProperties = renderProperties; }

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
	AbstractMapper()
	{
		objectProperties = properties.getObjectProperties();
	}

	virtual GLuint getShaderProgramID() = 0;

	virtual void update() = 0;

	// Binds the shader program
	virtual void use(Renderer* ren) = 0;

	virtual void draw(Renderer* ren) = 0;

protected:
	PropertyMap<32>* objectProperties = nullptr;
	ShaderProperties properties;
};