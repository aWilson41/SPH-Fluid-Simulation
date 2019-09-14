#include "AbstractMapper.h"
#include "Renderer.h"

unsigned long long ShaderProperties::update()
{
	// To quickly compile these bitset we add them
	key = std::bitset<64>(
		renderProperties.getPropertyBits().to_string() +
		sceneProperties.getPropertyBits().to_string() +
		objectProperties.getPropertyBits().to_string());
	renderProperties.makeCurrent();
	sceneProperties.makeCurrent();
	objectProperties.makeCurrent();
	keyLong = key.to_ullong();
	return keyLong;
}

std::string ShaderProperties::getFullPropertyString()
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

void AbstractMapper::use(Renderer* ren)
{
	if (!useCustomShader)
		useShader(ren->getShaderGroup());
}