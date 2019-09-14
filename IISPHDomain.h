#pragma once
#include "Particle.h"
#include <MathHelper.h>

// IISPH from the paper "Implicit Incompressible SPH" which describes the use of Relaxed Jacobi method to optimize densities
// for incompressible flow.
class IISPHDomain
{
public:
	// Sets the particles and initializes the bounds
	void initParticles(std::vector<SPHParticle> particles, glm::vec3 origin, glm::vec3 size, GLfloat bufferRatio = 0.1f);
	void initParticles(std::vector<IISPHParticle> particles, glm::vec3 origin, glm::vec3 size, GLfloat bufferRatio = 0.1f);

	void calcDensity();
	void calcNonPressureForces();
	void jacobiPressureSolve();
	void calcPressureForce();

	void update(GLfloat dt);
	void collision(glm::vec3 pos, glm::vec3& v);

public:
	std::vector<IISPHParticle> particles;

	glm::vec3 origin;
	glm::vec3 size;
	GLfloat bounds[6] = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f };

	GLfloat bufferBounds[6] = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f };
	glm::vec3 bufferSize;
	int gridWidth = -1;
	int gridHeight = -1;
	int gridDepth = -1;
};