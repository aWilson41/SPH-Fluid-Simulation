#pragma once
#include "Engine/MathHelper.h"

class Particle
{
public:
	Particle() { }
	Particle(glm::vec3* pos, GLfloat mass)
	{
		Particle::pos = pos;
		Particle::mass = mass;
	}

	void updateVelocity(GLfloat dt) { velocity += accel * dt; }
	void updatePos(GLfloat dt) { *pos += velocity * dt; }

	glm::vec3 getPos() { return *pos; }

public:
	GLfloat mass = -1.0f;
	GLfloat density = -1.0f;
	GLfloat pressure = -1.0f;
	glm::vec3* pos = nullptr;
	glm::vec3 velocity = glm::vec3(0.0f);
	glm::vec3 accel = glm::vec3(0.0f);

	std::vector<Particle*> neighbors;
};