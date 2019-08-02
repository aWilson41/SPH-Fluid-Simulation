#pragma once
#include "PropertyMap.h"
#include <GL/glew.h>
#include <string>
#include <vector>

class ShaderFile
{
public:
	ShaderFile() { }
	ShaderFile(std::string fileName, GLenum shaderType) { load(fileName, shaderType); }

public:
	void load(std::string fileName, GLenum shaderType);
	void compile();

	void setID(GLuint id) { shaderID = id; }

	std::string getSrc() { return srcStr; }
	std::string getFileName() { return fileName; }
	GLuint getShaderID() { return shaderID; }
	GLenum getShaderType() { return shaderType; }

private:
	std::string read();

private:
	GLuint shaderID = -1;
	std::string srcStr = "";
	std::string fileName = "";
	GLenum shaderType = GL_FRAGMENT_SHADER;
};


class ShaderProgram
{
public:
	ShaderProgram(std::string name) { shaderName = name; }
	~ShaderProgram();

public:
	std::string getName() { return shaderName; }

	std::string readShaderFile(std::string filePath);


	void loadShader(std::string filePath, GLenum shaderType);

	void compileShader(const char* src, GLuint shaderID, std::string filePath);
	void compileProgram();

	void release();

	GLuint getProgramID() { return programID; }

protected:
	std::string shaderName = "unnamed";
	GLuint programID = -1;

	std::vector<ShaderFile*> files;
};