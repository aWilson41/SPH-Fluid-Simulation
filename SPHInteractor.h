#pragma once
#include "Engine/TrackballCameraInteractor.h"

class GlyphPolyDataMapper;
class SPHDomain;
class SphereSource;

class SPHInteractor : public TrackballCameraInteractor
{
public:
	SPHInteractor();
	~SPHInteractor();

public:
	void keyDown(int key) override;
	void keyUp(int key) override;

	void update();
	void updateParticleMapper();

	GlyphPolyDataMapper* getParticleMapper() { return particleMapper; }

protected:
	SPHDomain* sphDomain = nullptr;
	SphereSource* sphereSource = nullptr;
	GlyphPolyDataMapper* particleMapper = nullptr;
	std::vector<std::tuple<GLfloat, glm::vec3>> colorFunc;
	UINT iter = 0;
	bool running = false;
};