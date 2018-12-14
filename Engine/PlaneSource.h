#pragma once
#include "MathHelper.h"

class PolyData;

// Defines a plane (defaults 1x1 with origin [0, 0, 0])
class PlaneSource
{
public:
	PlaneSource();
	~PlaneSource();

public:
	// Updates the output poly data to the plane specified by the parameters
	void update();

	PolyData* getOutput() { return outputData; }

	glm::vec3 getOrigin() { return origin; }
	glm::vec3 getP1() { return p1; }
	glm::vec3 getP2() { return p2; }

	void setOrigin(glm::vec3 origin) { PlaneSource::origin = origin; }
	void setP1(glm::vec3 p1) { PlaneSource::p1 = p1; }
	void setP2(glm::vec3 p2) { PlaneSource::p2 = p2; }

protected:
	PolyData* outputData = nullptr;

	glm::vec3 origin = glm::vec3(-0.5f, 0.0f, -0.5f);
	glm::vec3 p1 = glm::vec3(0.5f, 0.0f, -0.5f);
	glm::vec3 p2 = glm::vec3(-0.5f, 0.0f, 0.5f);
};