#pragma once

#include <vector>

class Particle
{
public:
	Particle()
	{

	}

public:
	vmath::vec3 pos;
	vmath::vec3 v;
	float density;
};