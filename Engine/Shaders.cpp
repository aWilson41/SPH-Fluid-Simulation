#include "Shaders.h"
#include "Renderer.h"
#include <string>
#include <fstream>

namespace Shaders
{
	std::vector<ShaderProgram*> shaderCache;

	/*ShaderProgram* getShader(std::string name)
	{
		for (unsigned int i = 0; i < shaders.size(); i++)
		{
			if (shaders[i]->getName() == name)
				return shaders[i];
		}
		return nullptr;
	}*/

	ShaderProgram* LoadVSFSShader(std::string shaderName, std::string vsPath, std::string fsPath)
	{
		ShaderProgram* shader = new ShaderProgram(shaderName);
		shader->loadVertexShader(vsPath);
		shader->loadFragmentShader(fsPath);
		shader->compileProgram();
		shaderCache.push_back(shader);
		return shader;
	}

	ShaderProgram* getShader(Renderer* ren, std::string mapperName, std::bitset<32> propertyKey)//, std::bitset<32> scenePropertyKey)
	{
		std::string shaderPath = "Shaders/" + ren->getShaderDirectory() + mapperName + '/';
		// Read the mapping file to find the correct shader
		//printf("Bitset: %s\n", propertyKey.to_string().c_str());
		unsigned long lineNum = propertyKey.to_ulong();
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
		return LoadVSFSShader(ren->getShaderDirectory() + mapperName + propertyKey.to_string(), shaderPath + vsShaderFileStr, shaderPath + fsShaderFileStr);
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