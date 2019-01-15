#pragma once
#include "Constants.h"
#include "Engine/TrackballCameraInteractor.h"

class GlyphPolyDataMapper;
class IISPHDomain;
class Renderer;
class SphereSource;
class SPHDomain;
class ThreadedSPHDomain;

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

	void setRenderer(Renderer* ren) { SPHInteractor::ren = ren; }
	GlyphPolyDataMapper* getParticleMapper() { return particleMapper; }

protected:
#ifdef IISPH
	IISPHDomain* sphDomain = nullptr;
#else
	#ifdef MULTITHREAD
		ThreadedSPHDomain* sphDomain = nullptr;
	#else
		SPHDomain* sphDomain = nullptr;
#endif
#endif

	Renderer* ren = nullptr;
	SphereSource* particleSphereSource = nullptr;
	GlyphPolyDataMapper* particleMapper = nullptr;
	std::vector<std::tuple<GLfloat, glm::vec3>> colorFunc;

	UINT iter = 0;
	bool running = false;
	bool writingFrames = true;

	GLfloat forceScale = 1.0f;
};