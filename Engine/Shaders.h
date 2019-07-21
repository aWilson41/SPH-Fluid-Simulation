#pragma once
#include "ShaderProgram.h"
#include <vector>
#include <bitset>

class Renderer;

namespace Shaders
{
	extern std::vector<ShaderProgram*> shaderCache;

	//extern ShaderProgram* getShader(std::string name);

	extern ShaderProgram* getShader(
		Renderer* ren, // Given the renderer type
		std::string mapperName, // The mapper type
		std::bitset<32> propertyKey);// , // The properties of the data (Ex: Render using vertex normals/colors/etc)
		//std::bitset<32> scenePropertyKey); // The properties of the scene (Ex: Render using cone light/point light/etc)

	extern void deleteShaders();
};