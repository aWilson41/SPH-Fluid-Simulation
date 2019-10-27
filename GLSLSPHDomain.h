#pragma once
#include "Particle.h"
#include <MathHelper.h>

class ShaderProgram;

struct GPUParticle
{
public:
	glm::vec3 position;
	GLfloat pad1;

	glm::vec3 velocity;
	GLfloat pad2;

	glm::vec3 acceleration;
	GLfloat pad3;

	GLfloat mass;
	GLfloat density;
	GLfloat pressure;
	GLfloat pad4;
};

class GLSLSPHDomain
{
public:
	GLSLSPHDomain();
	~GLSLSPHDomain();

public:
	// Sets the particles and initializes the bounds
	void initParticles(std::vector<SPHParticle> particles, glm::vec3 origin, glm::vec3 size, GLfloat bufferRatio = 0.1f);

	void update(GLfloat dt);

	void invokeBinProgram();
	void invokePressureProgram();
	void invokeForcesProgram();
	void invokeIntegrateProgram(GLfloat dt);

public:
	ShaderProgram* binProgram = nullptr;
	ShaderProgram* computePressureProgram = nullptr;
	ShaderProgram* computeForcesProgram = nullptr;
	ShaderProgram* integrateProgram = nullptr;
	std::vector<SPHParticle> particles;

	glm::vec3 origin = glm::vec3(0.0f);
	glm::vec3 size = glm::vec3(0.0f);
	GLfloat bounds[6] = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f };

	GLfloat bufferBounds[6] = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f };
	glm::vec3 bufferSize = glm::vec3(0.0f);
	int gridWidth = -1;
	int gridHeight = -1;
	int gridDepth = -1;
	int numVoxels = -1;

public:
	GPUParticle* gpuParticles = nullptr;
	GLuint particleBufferID = -1;

	std::vector<GLuint> bins;
	GLuint binBufferID = -1;
	GLuint neighborBufferID = -1;
	GLuint maxNeighbors = -1;
	GLuint maxParticlesPerCell = -1;
};