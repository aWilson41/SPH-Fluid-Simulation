#pragma once
#include "Engine/MathHelper.h"
#include "Particle.h"

class SPHDomain
{
public:
	// Sets the particles and initializes the bounds
	void initParticles(std::vector<Particle> particles, glm::vec3 origin, glm::vec3 size, GLfloat bufferRatio = 0.1f);

	// Calculate the neighbors
	void calcDensity();
	void calcForces();
	// 3d pos used for 
	void collision(glm::vec3 pos, glm::vec3& v);
	void update(GLfloat dt);

public:
	std::vector<Particle> particles;

	glm::vec3 origin;
	glm::vec3 size;
	GLfloat bounds[6] = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f };

	GLfloat bufferBounds[6] = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f };
	glm::vec3 bufferSize;
	int gridWidth = -1;
	int gridHeight = -1;
	int gridDepth = -1;
};