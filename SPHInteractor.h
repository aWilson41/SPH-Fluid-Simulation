#pragma once
#include "Constants.h"
#include <TrackballCameraInteractor.h>

class GlyphPolyDataMapper;
class Renderer;
class SphereSource;

#if IMPLEMENTATION == MULTI_THREADED
class ThreadedSPHDomain;
#elif IMPLEMENTATION == MULTI_THREADED_POOL
class ThreadPoolSPHDomain;
#elif IMPLEMENTATION == GLSL_COMPUTE_SHADER
class GLSLSPHDomain;
#elif IMPLEMENTATION == IISPH
class IISPHDomain;
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

	void setRenderer(Renderer* ren) { SPHInteractor::ren = ren; }
	GlyphPolyDataMapper* getParticleMapper() { return particleMapper; }

protected:
#if IMPLEMENTATION == MULTI_THREADED
	ThreadedSPHDomain* sphDomain = nullptr;
#elif IMPLEMENTATION == MULTI_THREADED_POOL
	ThreadPoolSPHDomain* sphDomain = nullptr;
#elif IMPLEMENTATION == GLSL_COMPUTE_SHADER
	GLSLSPHDomain* sphDomain = nullptr;
#elif IMPLEMENTATION == IISPH
	IISPHDomain* sphDomain = nullptr;
#else
	SPHDomain* sphDomain = nullptr;
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