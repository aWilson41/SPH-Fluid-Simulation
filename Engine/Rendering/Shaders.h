#pragma once
#include "ShaderProgram.h"
#include <vector>

class Renderer;
class ShaderProperties;

namespace Shaders
{
	extern std::vector<ShaderProgram*> shaderCache;

	//extern ShaderProgram* getShader(std::string name);

	extern ShaderProgram* loadVSFSShader(std::string shaderName, std::string vsPath, std::string fsPath);
	extern ShaderProgram* loadComputeShader(std::string shaderName, std::string path);
	extern ShaderProgram* loadComputeShader(std::string shaderName, std::vector<std::string> paths);

	extern ShaderProgram* getShader(std::string shaderGroup, std::string mapperName, ShaderProperties* properties);

	extern void deleteShaders();
};