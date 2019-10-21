#pragma once
#include "Particle.h"
#include <MathHelper.h>

class ShaderProgram;

struct GPUParticle
{
public:
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	float mass;
	float density;
	float pressure;
};

class GLSLSPHDomain
{
public:
	GLSLSPHDomain();

public:
	// Sets the particles and initializes the bounds
	void initParticles(std::vector<SPHParticle> particles, glm::vec3 origin, glm::vec3 size, GLfloat bufferRatio = 0.1f);

	void update(GLfloat dt);

	void invokeComputeShader(unsigned int taskID, GLfloat dt);

public:
	ShaderProgram* shaderProgram = nullptr;
	std::vector<SPHParticle> particles;

	glm::vec3 origin;
	glm::vec3 size;
	GLfloat bounds[6] = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f };

public:
	std::vector<GPUParticle> gpuParticles;
	GLuint particleBufferID = -1;
};