#pragma once
#include "ShaderProgram.h"
#include <vector>

class Renderer;

namespace Shaders
{
	extern std::vector<ShaderProgram*> shaderCache;

	//extern ShaderProgram* getShader(std::string name);

	extern ShaderProgram* getShader(Renderer* ren, std::string mapperName, ShaderProperties* properties);

	extern void deleteShaders();
};