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
	unsigned long long update()
	{
		// To quickly compile these bitset we add them
		key = std::bitset<64>(
			renderProperties.getPropertyBits().to_string() +
			sceneProperties.getPropertyBits().to_string() +
			objectProperties.getPropertyBits().to_string());
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

	std::string getFullBitString() { return key.to_string(); }

	std::string getFullPropertyString()
	{
		std::map<std::string, size_t> renderPropertyMap = renderProperties.getIndexMap();
		std::map<std::string, size_t> scenePropertyMap = sceneProperties.getIndexMap();
		std::map<std::string, size_t> objectPropertyMap = objectProperties.getIndexMap();
		std::string results = "Render Properties <Bit, name, state>:\n";
		for (std::map<std::string, size_t>::iterator i = renderPropertyMap.begin(); i != renderPropertyMap.end(); i++)
		{
			std::string state = (renderProperties.getProperty(i->first) == true) ? "true" : "false";
			results += std::to_string(i->second) + ' ' + i->first + ' ' + state + '\n';
		}
		results += "Scene Properties <Bit, name, state>:\n";
		for (std::map<std::string, size_t>::iterator i = scenePropertyMap.begin(); i != scenePropertyMap.end(); i++)
		{
			std::string state = (sceneProperties.getProperty(i->first) == true) ? "true" : "false";
			results += std::to_string(i->second) + ' ' + i->first + ' ' + state + '\n';
		}
		results += "Object Properties <Bit, name, state>:\n";
		for (std::map<std::string, size_t>::iterator i = objectPropertyMap.begin(); i != objectPropertyMap.end(); i++)
		{
			std::string state = (objectProperties.getProperty(i->first) == true) ? "true" : "false";
			results += std::to_string(i->second) + ' ' + i->first + ' ' + state + '\n';
		}
		results.pop_back();
		return results;
	}

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