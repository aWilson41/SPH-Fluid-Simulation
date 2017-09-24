// Implementation based on
// https://www.cs.cornell.edu/~bindel/class/cs5220-f11/code/sph.pdf 
// Cornell CS 5220
#pragma once

#include "Particle.h"
#include "vmath.h"

#include <vector>

std::vector<Particle> particle;

// Holds simulation parameters
struct Parameters
{
	int nFrames = 0;   // Number of frames
	float h = 0.0f;    // Particle size
	float rho0 = 0.0f; // Reference density
	float k = 0.0f;    // Bulk modulus
	float mu = 0.0f;   // Viscosity
	float g = 0.0f;    // Gravity strength
};

// Holds information of the current state of the simulation
struct State
{
	int n = 0;				// Number of particles
	float mass = 0.0f;		// Particle mass
	float* rho;				// Particle densities
	vmath::vec3* positions; // Particle positions
	float* vh;				// Particle half step velocities
	float* v;				// Particle full step velocities
	vmath::vec3* a;				// Particle accelerations
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
	void start(int nFrames)
	{
		for (int i = 0; i < nFrames; i++)
		{

		}
	}

	// Updates the density
	void calcDensity()
	{
		const float h = params.h;
		const float h2 = h * h;
		const float h8 = h2 * h2 * h2 * h2;
		const float c = 4 * state.mass / (M_PI * h8);

		// Compute every particle with every other particle
		for (int i = 0; i < state.n; i++)
		{
			state.rho[i] += 4 * state.mass / (M_PI * h2);
			for (int j = i + 1; j < state.n; j++)
			{
				vmath::vec3 dPos = state.positions[2 * i] - state.positions[2 * j];
				float r2 = dPos[0] * dPos[0] + dPos[1] * dPos[1];
				float z = h2 - r2;
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
		const float h2 = h * h;

		calcDensity();

		// Start with gravity and surface forces
		for (int i = 0; i < state.n; i++)
		{
			int index = 2 * i;
			state.a[index][0] = 0;
			state.a[index][1] = -params.g;
		}

		// Constants for interaction term
		float c0 = state.mass / (M_PI * h2 * h2);
		float cp = 15 * params.k;
		float cv = -40 * params.mu;

		// Compute the interaction forces
		for (int i = 0; i < state.n; i++)
		{
			const float rhoi = state.rho[i];
			for (int j = i + 1; j < state.n; j++)
			{
				vmath::vec3 dPos = state.positions[2 * i] - state.positions[2 * j];
				float r2 = dPos[0] * dPos[0] + dPos[1] * dPos[1];
				if (r2 < h2)
				{
					const float rhoj = state.rho[j];
					float q = sqrt(r2) / h;
					float u = 1 - q;
					float w0 = c0 * u / (rhoi * rhoj);
					float wp = w0 * cp * (rhoi + rhoj - 2 * params.rho0) * u / q;
					float wv = w0 * cv;
					vmath::vec3 dV = state.v[2 * i] - state.v[2 * j];
					state.a[2 * i] += wp * dPos + wv * dV;
					state.a[2 * j] -= wp * dPos + wv * dV;
				}
			}
		}
	}

private:
	Parameters params;
	State state;
};