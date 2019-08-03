#pragma once
#include "Engine/MathHelper.h"
#include "Particle.h"

class SPHDomain
{
public:
	// Sets the particles and initializes the bounds
	void initParticles(std::vector<SPHParticle> particles, glm::vec3 origin, glm::vec3 size, GLfloat bufferRatio = 0.1f);

	void calcDensity();
	void calcForces();
	void update(GLfloat dt);
	void collision(glm::vec3 pos, glm::vec3& v);

public:
	std::vector<SPHParticle> particles;

	glm::vec3 origin = glm::vec3(0.0f);
	glm::vec3 size = glm::vec3(0.0f);
	GLfloat bounds[6] = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f };

	GLfloat bufferBounds[6] = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f };
	glm::vec3 bufferSize = glm::vec3(0.0f);
	int gridWidth = -1;
	int gridHeight = -1;
	int gridDepth = -1;
};