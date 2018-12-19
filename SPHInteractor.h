#pragma once
#include "Constants.h"
#include "Engine/TrackballCameraInteractor.h"

class GlyphPolyDataMapper;
class SphereSource;

#ifdef MULTITHREAD
class ThreadedSPHDomain;
#else
class SPHDomain;
#endif

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
#ifdef MULTITHREAD
	ThreadedSPHDomain* sphDomain = nullptr;
#else
	SPHDomain* sphDomain = nullptr;
#endif
	SphereSource* sphereSource = nullptr;
	GlyphPolyDataMapper* particleMapper = nullptr;
	std::vector<std::tuple<GLfloat, glm::vec3>> colorFunc;
	UINT iter = 0;
	bool running = false;
	bool writingFrames = true;

#ifdef TIMER
	GLfloat totalSimTime = 0.0f;
#endif
};