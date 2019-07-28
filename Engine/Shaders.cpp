#include "Shaders.h"
#include "AbstractMapper.h"
#include "Renderer.h"
#include <fstream>
#include <string>

namespace Shaders
{
	std::vector<ShaderProgram*> shaderCache;

	// Load shader program from source
	ShaderProgram* loadVSFSShader(std::string shaderName, std::string vsPath, std::string fsPath)
	{
		ShaderProgram* shader = new ShaderProgram(shaderName);
		shader->loadVertexShader(vsPath);
		shader->loadFragmentShader(fsPath);
		shader->compileProgram();
		shaderCache.push_back(shader);
		return shader;
	}

	ShaderProgram* getShader(Renderer* ren, std::string mapperName, ShaderProperties* properties)
	{
		if (ren == nullptr || properties == nullptr)
			return nullptr;

		properties->update();

		std::string shaderPath = "Shaders/" + ren->getShaderDirectory() + mapperName + '/';
		// Read the mapping file to find the correct shader
		unsigned long long lineNum = properties->getKey();
		std::ifstream file;
		file.open(shaderPath + "mappings.csv");
		if (file.fail())
		{
			printf("Failed to read shader mappings file.\n");
			return nullptr;
		}
		file.seekg(std::ios::beg); // Start from beginning of the file
		for (unsigned long i = 0; i < lineNum; i++)
		{
			file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
		std::string vsShaderFileStr, fsShaderFileStr;
		vsShaderFileStr = fsShaderFileStr = "";
		std::getline(file, vsShaderFileStr, ',');
		std::getline(file, fsShaderFileStr, '\n');
		file.close();

		// Eventually this will be replaced with a replaceable shader system that maps the keys directly to the construction of the shader
		// so there is no required shader database
		return loadVSFSShader("unnamed", shaderPath + vsShaderFileStr, shaderPath + fsShaderFileStr);
	}

	void deleteShaders()
	{
		for (unsigned int i = 0; i < shaderCache.size(); i++)
		{
			shaderCache[i]->release();
			delete shaderCache[i];
		}
	}
};