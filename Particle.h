#pragma once
#include "Engine/MathHelper.h"

class Particle
{
public:
	Particle() { }
	Particle(glm::vec3* pos) { Particle::pos = pos; }
	Particle(const Particle& particle)
	{
		pos = particle.pos;
		velocity = particle.velocity;
		accel = particle.accel;
	}

	void updateVelocity(GLfloat dt) { velocity += accel * dt; }
	void updatePos(GLfloat dt) { *pos += velocity * dt; }

	glm::vec3 getPos() { return *pos; }

public:
	glm::vec3* pos = nullptr;
	glm::vec3 velocity = glm::vec3(0.0f);
	glm::vec3 accel = glm::vec3(0.0f);
};

class SPHParticle : public Particle
{
public:
	SPHParticle() { }
	SPHParticle(glm::vec3* pos, GLfloat mass) : Particle(pos) { SPHParticle::mass = mass; }
	SPHParticle(const SPHParticle& particle) : Particle(particle)
	{
		mass = particle.mass;
		density = particle.density;
		pressure = particle.pressure;
		neighbors = particle.neighbors;
		gridX = particle.gridX;
		gridY = particle.gridY;
		gridZ = particle.gridZ;
	}

public:
	GLfloat mass = -1.0f;
	GLfloat density = -1.0f;
	GLfloat pressure = -1.0f;

	std::vector<SPHParticle*> neighbors;
	int gridX = -1;
	int gridY = -1;
	int gridZ = -1;
};

class IISPHParticle : public SPHParticle
{
public:
	IISPHParticle() { }
	IISPHParticle(glm::vec3* pos, GLfloat mass) : SPHParticle(pos, mass) { }
	IISPHParticle(const IISPHParticle& particle) : SPHParticle(particle)
	{
		projectedDensity = particle.projectedDensity;
		pl = particle.pl;
		vAdv = particle.vAdv;
		dii = particle.dii;
		aii = particle.aii;
		dij_pj = particle.dij_pj;
	}
	IISPHParticle(const SPHParticle& particle) : SPHParticle(particle) { }

public:
	GLfloat projectedDensity = 0.0f;
	glm::vec3 vAdv = glm::vec3(0.0f);
	glm::vec3 dii = glm::vec3(0.0f);
	GLfloat aii = 0.0f;
	glm::vec3 dij_pj = glm::vec3(0.0f);
	GLfloat pl = 0.0f;
};