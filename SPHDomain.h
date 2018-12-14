#pragma once
#include "Engine/MathHelper.h"
#include "Particle.h"

class SPHDomain
{
public:
	// Sets the particles and initializes the bounds
	void initParticles(std::vector<Particle> particles, glm::vec3 origin, glm::vec3 size);

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

	// Stats
	GLfloat maxPressure = std::numeric_limits<GLfloat>::min();
	GLfloat minPressure = std::numeric_limits<GLfloat>::max();
	GLfloat maxPressureForce = std::numeric_limits<GLfloat>::min();
	GLfloat maxViscousForce = std::numeric_limits<GLfloat>::min();
	/*GLfloat maxDensity = 0.0f;
	glm::vec3 maxVelocity = glm::vec3(0.0f);
	glm::vec3 maxAccel = glm::vec3(0.0f);*/
};