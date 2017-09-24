// Implementation based on
// https://www.cs.cornell.edu/~bindel/class/cs5220-f11/code/sph.pdf 
// Cornell CS 5220
#pragma once

#include "vmath.h"

#include <vector>

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
	vmath::vec3* vh;		// Particle half step velocities
	vmath::vec3* v;			// Particle full step velocities
	vmath::vec3* a;			// Particle accelerations
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
		const float c = 4.0f * state.mass / (vmath::PI_F * h8);

		// Compute every particle with every other particle
		for (int i = 0; i < state.n; i++)
		{
			state.rho[i] += 4.0f * state.mass / (vmath::PI_F * h2);
			for (int j = i + 1; j < state.n; j++)
			{
				vmath::vec3 dPos = state.positions[i] - state.positions[j];
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
			state.a[i][0] = 0.0f;
			state.a[i][1] = -params.g;
		}

		// Constants for interaction term
		float c0 = state.mass / (vmath::PI_F * h2 * h2);
		float cp = 15.0f * params.k;
		float cv = -40.0f * params.mu;

		// Compute the interaction forces
		for (int i = 0; i < state.n; i++)
		{
			const float rhoi = state.rho[i];
			for (int j = i + 1; j < state.n; j++)
			{
				vmath::vec3 dPos = state.positions[i] - state.positions[j];
				float r2 = dPos[0] * dPos[0] + dPos[1] * dPos[1];
				if (r2 < h2)
				{
					const float rhoj = state.rho[j];
					float q = sqrt(r2) / h;
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
		const float XMAX = 1.0f;
		const float YMIN = 0.0f;
		const float YMAX = 1.0f;

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
		}
	}

	void dampReflect(int which, float barrier, vmath::vec3 pos, vmath::vec3 v, vmath::vec3 vh)
	{
		// Coefficient of resitiution
		const float DAMP = 0.75f;
		// Ignore degenerate cases
		if (v[which] == 0)
			return;
		// Scale back the distance traveled based on time from collision
		float tbounce = (pos[which] - barrier) / v[which];
		pos -= v * (1.0f - DAMP) * tbounce;
		// Reflect the position and velocity
		pos[which] = 2.0f * barrier - pos[which];
		v[which] = -v[which];
		vh[which] = -vh[which];
		v *= DAMP;
		vh *= DAMP;
	}

private:
	Parameters params;
	State state;
};