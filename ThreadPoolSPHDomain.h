#pragma once
#include "Particle.h"
#include <MathHelper.h>

class StdMultiThreaderPool;

class ThreadPoolSPHDomain
{
public:
	ThreadPoolSPHDomain();
	~ThreadPoolSPHDomain();

public:
	// Sets the particles and initializes the bounds
	void initParticles(std::vector<SPHParticle> particles, glm::vec3 origin, glm::vec3 size, GLfloat bufferRatio = 0.1f);

	void calcDensity(int threadID, int numThreads);
	void calcForces(int threadID, int numThreads);
	void integrate(int threadID, int numThreads);
	void update(GLfloat dt);
	void collision(glm::vec3 pos, glm::vec3& v);

public:
	StdMultiThreaderPool* threader;
	std::vector<SPHParticle> particles;
	std::vector<std::vector<SPHParticle*>> bins;
	GLfloat dt = -1.0f;

	glm::vec3 origin;
	glm::vec3 size;
	GLfloat bounds[6] = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f };

	GLfloat bufferBounds[6] = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f };
	glm::vec3 bufferSize;
	int gridWidth = -1;
	int gridHeight = -1;
	int gridDepth = -1;
};