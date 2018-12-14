#pragma once
#include "MathHelper.h"

class PolyData;

// Defines a uv sphere (defaults 0.5 radius for 1x1x1 sphere)
class SphereSource
{
public:
	SphereSource();
	~SphereSource();

public:
	// Updates the output poly data to the plane specified by the parameters
	void update();

	PolyData* getOutput() { return outputData; }

	glm::vec3 getOrigin() { return origin; }
	GLfloat getRadius() { return radius; }

	void setOrigin(glm::vec3 origin) { SphereSource::origin = origin; }
	void setRadius(GLfloat radius) { SphereSource::radius = radius; }

protected:
	PolyData* outputData = nullptr;

	glm::vec3 origin = glm::vec3(-0.5f, 0.0f, -0.5f);
	GLfloat radius = 0.5f;
	GLuint divTheta = 8;
	GLuint divPhi = 8;
};