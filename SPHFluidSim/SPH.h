// Implementation based on
// https://www.cs.cornell.edu/~bindel/class/cs5220-f11/code/sph.pdf 
// Cornell CS 5220
#pragma once

#include "vmath.h"

#include <vector>
#include <iostream>
#include <fstream>

// Holds simulation parameters
struct Parameters
{
	int nFrames = 2000;    // Number of frames
	int npFrames = 4;   // Number of steps per frame
	float h = 0.03f;      // Particle size
	float dt = 0.01f;   // Time step
	float rho0 = 100.0f; // Reference density
	float k = 100.0f;    // Bulk modulus
	float mu = 0.2f;      // Viscosity
	float g = 0.8f;       // Gravity strength
};
// We can alter npFrames to speed up simulation without increasing forces

// Holds information of the current state of the simulation
struct State
{
	int n = 0;              // Number of particles
	float mass = 0.0f;      // Particle mass
	float* rho;             // Particle densities
	float elapsedTime = 0.0f; // dt cummulation
	vmath::vec3* positions; // Particle positions
	vmath::vec3* vh;        // Particle half step velocities
	vmath::vec3* v;         // Particle full step velocities
	vmath::vec3* a;         // Particle accelerations
};

class SPH
{
public:
	SPH()
	{
		
	}

public:
	void setParameters(int nFrames, float particleSize, 
		float referenceDensity, float bulkModulus, 
		float viscosity, float g)
	{
		params.nFrames = nFrames;
		params.h = particleSize;
		params.rho0 = referenceDensity;
		params.k = bulkModulus;
		params.mu = viscosity;
		params.g = g;
	}

	void setParameters(Parameters params)
	{
		SPH::params = params;
	}

	// Number of frames
	void start()
	{
		float dt = params.dt;
		initParticles();
		calcForces();
		leapfrogStart(dt);
		writeParticles(0);
		for (int frame = 0; frame < params.nFrames; frame++)
		{
			for (int step = 0; step < params.npFrames; step++)
			{
				calcForces();
				leapfrogStep(dt);
				state.elapsedTime += dt;
			}
			writeParticles(frame + 1);
			printf("Frame %d Complete\n", frame);
			//printf("Time %f\n", state.elapsedTime);
		}

		printf("Simulation Complete");
	}

	// Updates the density
	void calcDensity()
	{
		const float h = params.h;
		const float h2 = h * h;
		const float h8 = h2 * h2 * h2 * h2;
		const float c = 4.0f * state.mass / (vmath::PI_F * h8);
		const float a = 4.0f * state.mass / (vmath::PI_F * h2);

		// For every particle i
		for (int i = 0; i < state.n; i++)
		{
			state.rho[i] += a;
			// For every particle j in front of i
			for (int j = i + 1; j < state.n; j++)
			{
				vmath::vec3 dPos = state.positions[i] - state.positions[j];
				float z = h2 - vmath::dot(dPos, dPos);
				if (z > 0)
				{
					float rho_ij = c * z * z * z;
					state.rho[i] += rho_ij;
					state.rho[j] += rho_ij;
				}
			}
		}
	}

	// Updates the acceleration
	void calcForces()
	{
		const float h = params.h;
		float invH = 1 / h;
		const float h2 = h * h;

		calcDensity();

		// Apply gravity to every particle
		for (int i = 0; i < state.n; i++)
		{
			state.a[i] = vmath::vec3(0.0f, -params.g, 0.0f);
		}

		// Constants for particle interaction
		float c0 = state.mass / (vmath::PI_F * h2 * h2);
		float cp = 15.0f * params.k;
		float cv = -40.0f * params.mu;

		// For every particle i
		for (int i = 0; i < state.n; i++)
		{
			// Get the density of particle i
			const float rhoi = state.rho[i];
			// For every particle j in front of i
			for (int j = i + 1; j < state.n; j++)
			{
				// Compute the difference in positions
				vmath::vec3 dPos = state.positions[i] - state.positions[j];
				// Scalar distance squared
				float r2 = vmath::dot(dPos, dPos);
				// If the distance squared is less than the particle size squared
				if (r2 < h2)
				{
					// Get the density of particle j
					const float rhoj = state.rho[j];
					// Dist between particles / particle size
					float q = sqrt(r2) * invH;
					float u = 1.0f - q;
					float w0 = c0 * u / (rhoi * rhoj);
					float wp = w0 * cp * (rhoi + rhoj - 2.0f * params.rho0) * u / q;
					float wv = w0 * cv;
					vmath::vec3 dV = state.v[i] - state.v[j];
					state.a[i] += wp * dPos + wv * dV;
					state.a[j] -= wp * dPos + wv * dV;
				}
			}
		}
	}

	// Integration
	void leapfrogStep(float dt)
	{
		for (int i = 0; i < state.n; i++)
		{
			state.vh[i] += state.a[i] * dt;
		}
		for (int i = 0; i < state.n; i++)
		{
			state.v[i] = state.vh[i] + state.a[i] * dt * 0.5f;
		}
		for (int i = 0; i < state.n; i++)
		{
			state.positions[i] += state.vh[i] * dt;
		}

		reflectBoundaryCondition();
	}

	// Integration start
	void leapfrogStart(float dt)
	{
		for (int i = 0; i < state.n; i++)
		{
			state.vh[i] = state.v[i] + state.a[i] * dt * 0.5f;
		}
		for (int i = 0; i < state.n; i++)
		{
			state.v[i] += state.a[i] * dt;
		}
		for (int i = 0; i < state.n; i++)
		{
			state.positions[i] += state.vh[i] * dt;
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

		for (int i = 0; i < state.n; i++)
		{
			if (state.positions[i][0] < XMIN)
				dampReflect(0, XMIN, state.positions[i], state.v[i], state.vh[i]);
			if (state.positions[i][0] > XMAX) // else if?
				dampReflect(0, XMAX, state.positions[i], state.v[i], state.vh[i]);
			if (state.positions[i][1] < YMIN)
				dampReflect(1, YMIN, state.positions[i], state.v[i], state.vh[i]);
			if (state.positions[i][1] > YMAX) // else if?
				dampReflect(1, YMAX, state.positions[i], state.v[i], state.vh[i]);
			if (state.positions[i][2] < ZMIN)
				dampReflect(2, ZMIN, state.positions[i], state.v[i], state.vh[i]);
			if (state.positions[i][2] > ZMAX)
				dampReflect(2, ZMAX, state.positions[i], state.v[i], state.vh[i]);
		}
	}

	void dampReflect(int which, float barrier, vmath::vec3& pos, vmath::vec3& v, vmath::vec3& vh)
	{
		// Coefficient of resitiution (elasticity)
		const float DAMP = 0.8f;
		const float E = 1.0f - DAMP;

		// If the velocity is 0 do nothing
		if (v[which] == 0.0f)
			return;

		// Distance from the barrier in the direction(which x, y, z) / velocity in the direction
		// How many updates of velocity do we need to travel this distance
		float tbounce = (pos[which] - barrier) / v[which];

		// Removing v * tbounce would cancel the v component putting it on the border.
		// We remove a fraction of that to dampen
		pos -= v * E * tbounce;
		// Reflect the position and velocity
		pos[which] = 2.0f * barrier - pos[which]; // ?
		v[which] = -v[which];
		vh[which] = -vh[which];
		v *= DAMP;
		vh *= DAMP;
	}

	static int boxFunc(float x, float y, float z)
	{
		const float XMIN = 0.0f;
		const float XMAX = 1.0f;
		const float YMIN = 0.0f;
		const float YMAX = 1.0f;
		const float ZMIN = 0.0f;
		const float ZMAX = 1.0f;
		return x < XMAX && y < YMAX && z < ZMAX && x > XMIN && y > YMIN && z > ZMIN && 1.0f - x > y;
	}

	typedef int(*domain_fun_t)(float, float, float);
	void fillRegion(domain_fun_t func)
	{
		float h = params.h;
		// Put the particles closer than their radius so they explode a bit in the beginning
		float hh = h / 1.1f;
		// Count mesh points that fall in indicated region.
		// For the box this is overkill but its fine
		int count = 0;
		for (float x = 0.0f; x < 1.0f; x += hh)
		{
			for (float y = 0.0f; y < 1.0f; y += hh)
			{
				for (float z = 0.0f; z < 1.0f; z += hh)
				{
					count += func(x, y, z);
				}
			}
		}

		printf("Particle count: %d\n", count);

		// Populate the particle data structure
		state.n = count;
		state.positions = new vmath::vec3[count];
		state.v = new vmath::vec3[count];
		state.vh = new vmath::vec3[count];
		state.a = new vmath::vec3[count];
		state.rho = new float[count];
		int p = 0;
		for (float x = 0.0f; x < 1.0f; x += hh)
		{
			for (float y = 0.0f; y < 1.0f; y += hh)
			{
				for (float z = 0.0f; z < 1.0f; z += hh)
				{
					if (func(x, y, z))
					{
						state.positions[p] = vmath::vec3(x, y, z);
						state.v[p] = vmath::vec3(0.0f, 0.0f, 0.0f);
						state.a[p] = vmath::vec3(0.0f, 0.0f, 0.0f);
						state.vh[p] = vmath::vec3(0.0f, 0.0f, 0.0f);
						state.rho[p++] = params.rho0;
					}
				}
			}
		}
	}

	void normalizeMass()
	{
		state.mass = 1.0f;
		calcDensity();
		float rho2s = 0.0f;
		float rhos = 0.0f;
		for (int i = 0; i < state.n; i++)
		{
			rho2s += state.rho[i] * state.rho[i];
			rhos += state.rho[i];
		}
		state.mass *= params.rho0 * rhos / rho2s;
	}

	void initParticles()
	{
		fillRegion(boxFunc);
		normalizeMass();
	}

	// Simply writes every particle to a file
	void writeParticles(int frame)
	{
		std::ofstream file;
		file.open("output/Test" + std::to_string(frame) +".dat");
		file << state.n << " "; // Number of particles
		for (unsigned int i = 0; i < state.n; i++)
		{
			file << state.positions[i][0] - 0.5f << " " << state.positions[i][1] - 0.5f << " " << state.positions[i][2] - 0.5f << " ";
			float v = state.v[i][0] * state.v[i][0] + state.v[i][1] * state.v[i][1] + state.v[i][2] * state.v[i][2];
			v = sqrt(v);
			file << v << " ";
		}
		file.close();
	}

	void cleanup()
	{
		delete[] state.positions;
		delete[] state.v;
		delete[] state.vh;
		delete[] state.a;
	}

private:
	Parameters params;
	State state;
};