#pragma once

#include "Particle.h"
#include "vmath.h"

#include <vector>

std::vector<Particle> particle;

class SPH
{
public:
	SPH()
	{
		
	}

public:
	void setParamters(int nFrames, float particleSize, 
		float referenceDensity, float bulkModulus, 
		float viscosity, float g)
	{
		SPH::nFrames = nFrames;
		h = particleSize;
		rho0 = referenceDensity;
		k = bulkModulus;
		mu = viscosity;
		SPH::g = g;
	}

	// Number of frames
	void start(int nFrames)
	{
		for (int i = 0; i < nFrames; i++)
		{

		}
	}

private:
	int nFrames = 0;
	float h = 0.0f;
	float rho0 = 0.0f;
	float k = 0.0f;
	float mu = 0.0f;
	float g = 0.0f;
};