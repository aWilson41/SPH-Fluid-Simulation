#pragma once

// Remove debug iterators
#define _ITERATOR_DEBUG_LEVEL 0

#include "vmath.h"

#include <vector>
#include <iostream>
#include <fstream>
//#include <cstdio>
#include <ctime>
//#include <conio.h>

class Particle
{
public:
	vmath::vec3 position; // Particle position
	vmath::vec3 velocity; // Particle full step velocity
	vmath::vec3 velocityHalf; // Half step
	vmath::vec3 accel;
	float pressure;
	float density;
	float mass;
};

class SPH
{
public:
	// Number of frames
	void start()
	{
		double totalTime = 0.0;
		initParticles(boxFunc);

		// Only needed to start leapfrog
		calcNeighbors();
		calcDensity();
		calcForces();
		leapFrogStart(dt);

		writeParticles(0);
		for (int frame = 0; frame < nFrames; frame++)
		{
			// Start a timer
			std::clock_t start;
			double duration = 0.0;
			start = std::clock();
			for (int step = 0; step < npFrames; step++)
			{
				calcNeighbors();
				calcDensity();
				calcForces();
				//integrate(dt);
				leapFrogIntegrate(dt);

				elapsedTime += dt;
			}
			writeParticles(frame + 1);
			//writeSurface(frame + 1);
			duration = (std::clock() - start) / static_cast<double>(CLOCKS_PER_SEC);
			totalTime += duration;
			printf("Frame %d Complete in %f seconds.\n", frame, duration);
		}	

		printf("Simulation Complete in %f secs\n", totalTime);
		printf("Average frame time %f secs\n", totalTime / nFrames);
		printf("Average step time %f secs\n", totalTime / (nFrames * npFrames));
	}

	// Calculate the neighbors
	void calcNeighbors()
	{
		const float h2 = h * h;
		for (int i = 0; i < n; i++)
		{
			Particle* p1 = particles[i];
			neighbors[i].clear();
			for (int j = 0; j < n /*&& i != j*/; j++)
			{
				Particle* p2 = particles[j];
				vmath::vec3 dist = p1->position - p2->position;
				float r2 = vmath::dot(dist, dist);
				if (r2 <= h2)
					neighbors[i].push_back(p2);
			}
		}
	}

	// Updates the density
	void calcDensity()
	{
		for (int i = 0; i < n; i++)
		{
			Particle* p1 = particles[i];
			float densitySum = 0.0f;

			for (int j = 0; j <  neighbors[i].size(); j++)
			{
				Particle* p2 = neighbors[i][j];
				vmath::vec3 dist = p1->position - p2->position;
				densitySum += p2->mass * kernel(dist);
			}
			// If the value is smaller than the rest density the pressure is just 0
			if (densitySum < restDensity)
				p1->density = restDensity;
			else
				p1->density = densitySum;
			p1->pressure = stiffness * (p1->density - restDensity);
		}
	}
	
	// Updates the acceleration
	void calcForces()
	{
		vmath::vec3 fGravity = vmath::vec3(0.0f, -g, 0.0f);
		for (int i = 0; i < n; i++)
		{
			Particle* p1 = particles[i];
			vmath::vec3 fPressure = vmath::vec3(0.0f, 0.0f, 0.0f);
			vmath::vec3 fViscosity = vmath::vec3(0.0f, 0.0f, 0.0f);

			for (int j = 0; j < neighbors[i].size(); j++)
			{
				Particle* p2 = neighbors[i][j];
				/*if (p1 == p2)
					continue;*/
				vmath::vec3 dist = p1->position - p2->position;

				// Pressure force density
				fPressure += p2->mass * (p2->pressure + p1->pressure) / (2.0f * p2->density) * gradKernel(dist);

				// Viscosity force density
				fViscosity += p2->mass * (p2->velocity - p1->velocity) / p2->density * laplaceKernel(dist);
			}

			fPressure *= -1.0f;
			fViscosity *= viscosity;

			p1->accel = (fPressure + fViscosity + p1->density * fGravity /* + fSurface*/) / p1->density;
		}
	}

	// Poly6 Kernel
	float kernel(vmath::vec3 x)
	{
		float r2 = vmath::dot(x, x);
		if (r2 < 0.0f || r2 > h2)
			return 0.0f;

		float l = h2 - r2;
		return c0 * l * l * l;
	}

	// Gradient of Spiky Kernel
	vmath::vec3 gradKernel(vmath::vec3 x)
	{
		float r = vmath::length(x);
		if (r == 0.0f)
			return vmath::vec3(0.0f, 0.0f, 0.0f);

		vmath::vec3 t2 = x / r;
		float l = h - r;

		return c1 * t2 * l * l;
	}

	// Laplacian of Viscosity Kernel
	float laplaceKernel(vmath::vec3 x)
	{
		float r = vmath::length(x);
		return c2 * (h - r);
	}

	
	// Integrates using the Euler method
	void integrate(float dt)
	{
		for (int i = 0; i < n; i++)
		{
			Particle* p = particles[i];
			p->velocity += dt * p->accel;
			p->position += dt * p->velocity;
		}

		reflectBoundaryCondition();
	}

	// Integrates using the leapfrog method
	void leapFrogIntegrate(float dt)
	{
		for (int i = 0; i < n; i++)
		{
			Particle* p = particles[i];
			p->velocityHalf += p->accel * dt;
			p->velocity = p->velocityHalf + p->accel * dt * 0.5f;
			p->position += p->velocityHalf * dt;
		}

		reflectBoundaryCondition();
	}

	// Should be called after first iteration
	void leapFrogStart(float dt)
	{
		for (int i = 0; i < n; i++)
		{
			Particle* p = particles[i];
			p->velocityHalf = p->velocity + p->accel * dt * 0.5f;
			p->velocity += p->accel * dt;
			p->position += p->velocityHalf * dt;
		}

		reflectBoundaryCondition();
	}


	// Enforces boundary conditions
	void reflectBoundaryCondition()
	{
		// Boundaries
		const float XMIN = 0.0f;
		//float XMIN = sin(state.elapsedTime * 0.5f) * 0.5f;
		const float XMAX = 3.0f;
		const float YMIN = 0.0f;
		const float YMAX = 1.0f;
		const float ZMIN = 0.0f;
		const float ZMAX = 1.0f;

		//const float XMIN = 0.0f;
		////float XMIN = sin(state.elapsedTime * 0.5f) * 0.5f;
		//const float XMAX = 1.5f;
		//const float YMIN = 0.0f;
		//const float YMAX = 0.75f;
		//const float ZMIN = 0.25f;
		//const float ZMAX = 0.75f;

		for (int i = 0; i < n; i++)
		{
			if (particles[i]->position[0] < XMIN)
				dampReflect(0, XMIN, particles[i]);
			if (particles[i]->position[0] > XMAX) // else if?
				dampReflect(0, XMAX, particles[i]);
			if (particles[i]->position[1] < YMIN)
				dampReflect(1, YMIN, particles[i]);
			if (particles[i]->position[1] > YMAX) // else if?
				dampReflect(1, YMAX, particles[i]);
			if (particles[i]->position[2] < ZMIN)
				dampReflect(2, ZMIN, particles[i]);
			if (particles[i]->position[2] > ZMAX)
				dampReflect(2, ZMAX, particles[i]);
		}
	}

	void dampReflect(int which, float barrier, Particle* p)
	{
		// Coefficient of resitiution (elasticity)
		//const float DAMP = 0.2f;
		//const float E = 1.0f - DAMP;

		//// If the velocity is 0 do nothing
		//if (p->velocity[which] == 0.0f)
		//	return;

		//// Distance from the barrier in the direction(which x, y, z) / velocity in the direction
		//// How many updates of velocity do we need to travel this distance
		//float tbounce = (p->position[which] - barrier) / p->velocity[which];

		//// Removing v * tbounce would cancel the v component putting it on the border.
		//// We remove a fraction of that to dampen
		//p->position -= p->velocity * E * tbounce;
		//// Reflect the position and velocity
		//p->position[which] = 2.0f * barrier - p->position[which]; // ?
		//p->velocity[which] = -p->velocity[which];
		//p->velocity *= DAMP;

		// We use a small restitution saving some of the velocity in the direction
		p->velocity[which] = -p->velocity[which] * 0.2f;
		p->velocityHalf[which] = -p->velocityHalf[which] * 0.2f;
		// Resolve the position as well
		p->position[which] = barrier;
		p->accel[which] = 0.0f;
	}


	// Functions for fluid initialization (domain is x, y, z grid (0,1), (0,1), (0,1))
	static bool boxFunc(float x, float y, float z)
	{
		/*const float XMIN = 0.0f;
		const float XMAX = 1.0f;
		const float YMIN = 0.0f;
		const float YMAX = 0.75f;
		const float ZMIN = 0.0f;
		const float ZMAX = 1.0f;*/

		const float XMIN = 0.0f;
		const float XMAX = 0.5f;
		const float YMIN = 0.0f;
		const float YMAX = 0.8f;
		const float ZMIN = 0.0f;
		const float ZMAX = 1.0f;
		return x < XMAX && y < YMAX && z < ZMAX && x > XMIN && y > YMIN && z > ZMIN;
	}

	static bool triangularPrismFunc(float x, float y, float z)
	{
		const float XMIN = 0.0f;
		const float XMAX = 1.0f; //0.5f;
		const float YMIN = 0.0f;
		const float YMAX = 0.75f; //0.5f;
		const float ZMIN = 0.0f; //0.25f;
		const float ZMAX = 1.0f; //0.75f;
		return x < XMAX && y < YMAX && z < ZMAX && x > XMIN && y > YMIN && z > ZMIN && 1.0f - x > y;
	}

	typedef bool(*domain_fun_t)(float, float, float);
	void initParticles(domain_fun_t func)
	{
		// Put the particles closer than their radius so they explode a bit in the beginning
		float hh = h / 1.25f;
		// Count mesh points that fall in indicated region.
		// For the box this is overkill but its fine
		particles = std::vector<Particle*>();
		unsigned int count = 0;
		for (float x = 0.0f; x < 1.0f; x += hh)
		{
			for (float y = 0.0f; y < 1.0f; y += hh)
			{
				for (float z = 0.0f; z < 1.0f; z += hh)
				{
					if (func(x, y, z))
					{
						count++;
						Particle* particle = new Particle();
						particle->position = vmath::vec3(x, y, z);
						particle->velocity = vmath::vec3(0.0f, 0.0f, 0.0f);
						particle->velocityHalf = vmath::vec3(0.0f, 0.0f, 0.0f);
						particle->accel = vmath::vec3(0.0f, 0.0f, 0.0f);
						particle->density = 0.0f;
						particle->pressure = 0.0f;
						particle->mass = initMass;
						particles.push_back(particle);
					}
				}
			}
		}
		n = count;
		neighbors = std::vector<std::vector<Particle*>>(n);

		printf("Particle count: %d\n", count);
	}


	// Simply writes every particle to a file
	void writeParticles(int frame)
	{
		std::ofstream file;
		file.open("particles/Test" + std::to_string(frame) +".dat");
		file << n << " "; // Number of particles
		for (int i = 0; i < n; i++)
		{
			Particle* p = particles[i];
			file << p->position[0] - 0.5f << " " << p->position[1] - 0.5f << " " << p->position[2] - 0.5f << " ";
			float v = vmath::length(p->velocity);
			//float v = p->density;
			file << v << " ";
		}
		file.close();
	}

	void cleanup()
	{
		for (int i = 0; i < particles.size(); i++)
		{
			delete particles[i];
		}
	}

private:
	int n = 0; // Number of particles
	float elapsedTime = 0.0f; // dt acummulation
	std::vector<Particle*> particles;
	std::vector<std::vector<Particle*>> neighbors; // Neighbors

public:
	int nFrames = 1000;          // Number of frames
	int npFrames = 2;            // Number of steps per frame
	float h = 0.03f;             // Particle size
	float h2 = h * h;
	float h4 = h2 * h2;
	float h6 = h4 * h2;
	float h9 = h6 * h2 * h;
	float c0 = 315.0f / (64.0f * static_cast<float>(M_PI) * h9);
	float c1 = -45.0f / (static_cast<float>(M_PI) * h6);
	float c2 = -c1;

	float dt = 0.02f;            // Time step
	float viscosity = 2.8f;      // Viscosity
	float g = 0.8f;             // Gravity strength
	float initMass = 0.01f;       // Mass assigned to each particle initially volume * restDensity / numParticles
	float stiffness = 150.5f;    // Internal pressure
	float restDensity = 998.29f; // Rest density, reference density, rho0
};