#include "ShaderProgram.h"
#include <fstream>
#include <sstream>

void printShaderError(GLuint shaderID)
{
	GLint logSize = 0;
	glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logSize);
	GLchar* data = new GLchar[logSize];
	glGetShaderInfoLog(shaderID, logSize, &logSize, data);
	printf("%s\n", data);
	delete[] data;

}
void printLinkerError(GLuint programID)
{
	GLint logSize = 0;
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logSize);
	GLchar* data = new GLchar[logSize];
	glGetProgramInfoLog(programID, logSize, &logSize, data);
	printf("%s\n", data);
	delete[] data;
}


void ShaderFile::load(std::string fileName, GLenum shaderType)
{
	ShaderFile::fileName = fileName;
	ShaderFile::shaderType = shaderType;
	srcStr = read();
}

std::string ShaderFile::read()
{
	// Read the shader
	std::ifstream file(fileName.c_str());
	if (file.fail())
	{
		printf(("Failed to open: " + fileName + "\n").c_str());
		return "";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

void ShaderFile::compile()
{
	const char* src = srcStr.c_str();
	glShaderSource(shaderID, 1, &src, NULL);
	glCompileShader(shaderID);
	GLint compile_ok = GL_FALSE;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compile_ok);
	if (!compile_ok)
	{
		printf(("Shader error in: " + fileName + "\n").c_str());
		printShaderError(shaderID);
		glDeleteShader(shaderID);
	}
	/*else
		printf((filePath + " compiled OK.\n").c_str());*/
}


ShaderProgram::~ShaderProgram()
{
	for (size_t i = 0; i < files.size(); i++)
	{
		delete files[i];
	}
	release();
}
void ShaderProgram::loadShader(std::string filePath, GLenum shaderType)
{
	ShaderFile* shaderFile = new ShaderFile(filePath, shaderType);
	files.push_back(shaderFile);
}
void ShaderProgram::compileProgram()
{
	for (size_t i = 0; i < files.size(); i++)
	{
		files[i]->setID(glCreateShader(files[i]->getShaderType()));
		files[i]->compile();
	}

	programID = glCreateProgram();
	for (size_t i = 0; i < files.size(); i++)
	{
		glAttachShader(programID, files[i]->getShaderID());
	}
	glLinkProgram(programID);

	// We then can delete the shaders, must be detached for deletion
	for (size_t i = 0; i < files.size(); i++)
	{
		glDetachShader(programID, files[i]->getShaderID());
		glDeleteShader(files[i]->getShaderID());
	}
	 
	GLint link_ok = GL_FALSE;
	glGetProgramiv(programID, GL_LINK_STATUS, &link_ok);
	if (!link_ok)
	{
		printf("Shader link error: ");
		printLinkerError(programID);
		glDeleteProgram(programID);
	}
	//else
	//	printf("Shader linked OK.\n");
}

void ShaderProgram::release()
{
	for (size_t i = 0; i < files.size(); i++)
	{
		glDetachShader(programID, files[i]->getShaderID());
		glDeleteShader(files[i]->getShaderID());
	}
	glDeleteProgram(programID);
}