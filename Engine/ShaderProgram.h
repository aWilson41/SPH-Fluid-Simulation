#pragma once
#include "PropertyMap.h"
#include <GL/glew.h>
#include <string>

class ShaderProgram
{
public:
	ShaderProgram(std::string name) { shaderName = name; }

public:
	std::string getName() { return shaderName; }

	std::string readShaderFile(std::string filePath);

	void loadVertexShader(std::string filePath);
	void loadFragmentShader(std::string filePath);
	void compileShader(const char* src, GLuint shaderID, std::string filePath);
	void compileProgram();

	void printShaderError(GLuint shaderID);
	void printLinkerError(GLuint programID);

	void release();

	GLuint getProgramID() { return programID; }

protected:
	std::string shaderName = "none";
	GLuint programID = -1;
	GLuint vertexShaderID = -1;
	GLuint fragShaderID = -1;

	std::string vertexFileName = "";
	std::string fragFileName = "";
	std::string vertexShaderSrc = "";
	std::string fragShaderSrc = "";
};

// Constructs the shader through database lookup using the properties
class DBShaderProgram : public ShaderProgram
{
public:
	DBShaderProgram(std::string name) : ShaderProgram(name) { }

	void lookupAndCreate(unsigned long long key);

protected:
	unsigned long long key = 0;
};