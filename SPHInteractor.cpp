#include "SPHInteractor.h"
#include "SPHRasterizer.h"

#if IMPLEMENTATION == MULTI_THREADED
#include "ThreadedSPHDomain.h"
#elif IMPLEMENTATION == MULTI_THREADED_POOL
#include "ThreadPoolSPHDomain.h"
#elif IMPLEMENTATION == GLSL_COMPUTE_SHADER
#include "GLSLSPHDomain.h"
#elif IMPLEMENTATION == IISPH
#include "IISPHDomain.h"
#else
#include "SPHDomain.h"
#endif

#include <chrono>
#include <Geometry3D.h>
#include <GLFW/glfw3.h>
#include <GlyphPolyDataMapper.h>
#include <ImageData.h>
#include <PNGWriter.h>
#include <RawImageWriter.h>
#include <Renderer.h>
#include <SphereSource.h>
#include <TrackballCamera.h>
#include <tuple>

SPHInteractor::SPHInteractor()
{
	// Set the particle positions
	std::vector<glm::vec3> particlePos;
	GLfloat iterLength = h / 1.5f; // Squish the particles together a bit for initialization

	// Sphere initialization
	/*glm::vec3 center = glm::vec3(0.0f, 0.5f, 0.0f);
	geom3d::Rect bounds = geom3d::Rect(glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(1.5f, 1.0f, 1.5f));
	glm::vec3 start = bounds.origin();
	glm::vec3 end = start + bounds.size();
	for (GLfloat x = start.x; x < end.x; x += iterLength)
	{
		for (GLfloat y = start.y; y < end.y; y += iterLength)
		{
			for (GLfloat z = start.z; z < end.z; z += iterLength)
			{
				glm::vec3 pos = glm::vec3(x + iterLength * 0.5f, y + iterLength * 0.5f, z + iterLength * 0.5f);
				glm::vec3 dist = center - pos;
				if (glm::dot(dist, dist) < 0.5f * 0.5f)
					particlePos.push_back(pos);
			}
		}
	}*/

	// Rect
	geom3d::Rect bounds = geom3d::Rect(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 2.0f, 1.0f));
	glm::vec3 start = bounds.origin();
	glm::vec3 end = start + bounds.size();
	for (GLfloat x = start.x; x < end.x; x += iterLength)
	{
		for (GLfloat y = start.y; y < end.y; y += iterLength)
		{
			for (GLfloat z = start.z; z < end.z; z += iterLength)
			{
				if (x < 0.5f && x > -0.5f &&
					y < 1.5f && y > 0.0f &&
					z < 0.5f && z > -0.5f)
					particlePos.push_back(glm::vec3(x, y, z));
			}
		}
	}

	printf("Num Particles: %d\n", static_cast<UINT>(particlePos.size()));
	// Create a uv sphere source for instancing
	particleSphereSource = new SphereSource();
	particleSphereSource->setRadius(h * 0.5f);
	particleSphereSource->update();
	// Create the particle mapper
	particleMapper = new GlyphPolyDataMapper();
	particleMapper->setInput(particleSphereSource->getOutput());
	particleMapper->allocateOffsets(static_cast<UINT>(particlePos.size()));
	particleMapper->allocateColorData(static_cast<UINT>(particlePos.size()));
	glm::vec3* offsetData = reinterpret_cast<glm::vec3*>(particleMapper->getOffsetData());
	glm::vec3* colorData = reinterpret_cast<glm::vec3*>(particleMapper->getColorData());
	// Set the offset data of particle mapper with the generate positions and create the particles
	std::vector<SPHParticle> particles(particlePos.size());
	for (UINT i = 0; i < particlePos.size(); i++)
	{
		offsetData[i] = particlePos[i];
		particles[i] = SPHParticle(&offsetData[i], PARTICLE_MASS);
	}

	// Setup the color function to use for the particles
	colorFunc.push_back(std::tuple<GLfloat, glm::vec3>(0.0f, glm::vec3(0.0f, 0.25f, 0.45f)));
	colorFunc.push_back(std::tuple<GLfloat, glm::vec3>(0.4f, glm::vec3(0.15f, 0.65f, 0.75f)));
	colorFunc.push_back(std::tuple<GLfloat, glm::vec3>(1.0f, glm::vec3(0.15f, 0.65f, 0.75f)));
	colorFunc.push_back(std::tuple<GLfloat, glm::vec3>(1.5f, glm::vec3(1.0f, 1.0f, 1.0f)));

	// Setup the SPHDomain for simulation
#if IMPLEMENTATION == MULTI_THREADED
	sphDomain = new ThreadedSPHDomain();
#elif IMPLEMENTATION == MULTI_THREADED_POOL
	sphDomain = new ThreadPoolSPHDomain();
#elif IMPLEMENTATION == GLSL_COMPUTE_SHADER
	sphDomain = new GLSLSPHDomain();
#elif IMPLEMENTATION == IISPH
	sphDomain = new IISPHDomain();
#else
	sphDomain = new SPHDomain();
#endif

#ifdef STARTIMMEDIATELY
	running = true;
#endif

	sphDomain->initParticles(particles, bounds.origin() - bounds.size() * 0.5f * glm::vec3(1.0f, 0.0f, 1.0f), bounds.size() * glm::vec3(2.0f, 5.0f, 2.0f));// +glm::vec3(1.5f, 0.0f, 0.0f));
	updateParticleMapper();
}
SPHInteractor::~SPHInteractor()
{
	if (particleSphereSource != nullptr)
		delete particleSphereSource;
	if (particleMapper != nullptr)
		delete particleMapper;
	if (sphDomain != nullptr)
		delete sphDomain;
}

void SPHInteractor::keyDown(int key)
{
	if (key == GLFW_KEY_P)
		writingFrames = !writingFrames;
	else if (key == GLFW_KEY_ENTER)
		running = !running;
	else if (key == GLFW_KEY_2)
	{
		forceScale += 1.0f;
		printf("hit\n");
	}
	else if (key == GLFW_KEY_1)
		forceScale -= 1.0f;
}
void SPHInteractor::keyUp(int key) { }

void SPHInteractor::update()
{
#ifdef TIMER
	auto start = std::chrono::steady_clock::now();
#endif
	if (running)
	{
		// Do the actual simulation
		for (UINT i = 0; i < SUBSTEPS; i++)
		{
			sphDomain->update(TIMESTEP);
		}
	}

#ifdef TIMER
	auto end = std::chrono::steady_clock::now();
	printf("Time Frame Took: %fs\n", std::chrono::duration<double, std::milli>(end - start).count() / 1000.0);
#endif

	updateParticleMapper();

	if (rightButtonDown)
	{
		geom3d::Ray ray = cam->getEyeRay(mousePos.x * 2.0f - 1.0f, 1.0f - mousePos.y * 2.0f);
		for (UINT i = 0; i < particleMapper->getInstanceCount(); i++)
		{
			if (geom3d::intersectSphereRay(geom3d::Sphere(*sphDomain->particles[i].pos, r), ray))
			{
				sphDomain->particles[i].velocity += ray.dir * forceScale;
				// All the particles nearby this one
				for (UINT j = 0; j < sphDomain->particles[i].neighbors.size(); j++)
				{
					sphDomain->particles[i].neighbors[j]->velocity += ray.dir * forceScale;
				}
			}
		}
	}

#ifdef OUTPUTFRAMES
	if (running && writingFrames && iter < NUMFRAMES)
	{
		// Get the frame
		GLint vp[4];
		glGetIntegerv(GL_VIEWPORT, vp);
		ImageData image;
		UINT dim[3] = { static_cast<UINT>(vp[2]), static_cast<UINT>(vp[3]), 1 };
		double spacing[3] = { 1.0, 1.0, 1.0 };
		double origin[3] = { 0.0, 0.0, 0.0 };
		image.allocate2DImage(dim, spacing, origin, 3, ScalarType::UCHAR_T);
		glReadPixels(0, 0, dim[0], dim[1], GL_RGB, GL_UNSIGNED_BYTE, image.getData());
		// Write the frame as png
		PNGWriter writer;
		if (iter < 10)
			writer.setFileName("output/frame_000" + std::to_string(iter) + ".png");
		else if (iter < 100)
			writer.setFileName("output/frame_00" + std::to_string(iter) + ".png");
		else if (iter < 1000)
			writer.setFileName("output/frame_0" + std::to_string(iter) + ".png");
		else if (iter < 10000)
			writer.setFileName("output/frame_" + std::to_string(iter) + ".png");
		writer.setInput(&image);
		writer.update();
		iter++;
	}
#endif
}

void SPHInteractor::updateParticleMapper()
{
	// Updates the colors of the particles to the velocities
	glm::vec3* colors = reinterpret_cast<glm::vec3*>(particleMapper->getColorData());
	for (UINT i = 0; i < particleMapper->getInstanceCount(); i++)
	{
		//colors[i] = glm::vec3(0.2f, 0.2f, 0.9f);
		colors[i] = MathHelp::lerp(colorFunc, glm::length(sphDomain->particles[i].velocity) / 1.5f);
	}
	particleMapper->update();
}