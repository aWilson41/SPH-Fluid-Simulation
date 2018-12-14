#include "SPHInteractor.h"
#include "Constants.h"
#include "Engine/Geometry3D.h"
#include "Engine/GlyphPolyDataMapper.h"
#include "Engine/Renderer.h"
#include "Engine/SphereSource.h"
#include "Engine/SphereSource.h"
#include "Particle.h"
#include "SPHDomain.h"
#include <chrono>
#include <tuple>

SPHInteractor::SPHInteractor()
{
	std::vector<glm::vec3> particlePos;
	GLfloat iterLength = h / 1.2f;
	for (GLfloat x = -1.0f; x < 1.0f; x += iterLength)
	{
		for (GLfloat y = -1.0f; y < 1.0f; y += iterLength)
		{
			for (GLfloat z = -1.0f; z < 1.0f; z += iterLength)
			{
				if (x < 0.25f && y < 0.5f && z < 0.25f && x > -0.25f && y > 0.0f && z > -0.25f)
					particlePos.push_back(glm::vec3(x, y, z));
			}
		}
	}

	sphereSource = new SphereSource();
	sphereSource->setRadius(h * 0.5f);
	sphereSource->update();
	particleMapper = new GlyphPolyDataMapper();
	particleMapper->setInput(sphereSource->getOutput());
	particleMapper->allocateOffsets(static_cast<UINT>(particlePos.size()));
	particleMapper->allocateColorData(static_cast<UINT>(particlePos.size()));
	glm::vec3* offsetData = reinterpret_cast<glm::vec3*>(particleMapper->getOffsetData());
	std::vector<Particle> particles(particlePos.size());
	for (UINT i = 0; i < particlePos.size(); i++)
	{
		offsetData[i] = particlePos[i];
		particles[i] = Particle(&offsetData[i], PARTICLE_MASS);
	}

	// Setup the color function to use for the particles
	colorFunc.push_back(std::tuple<GLfloat, glm::vec3>(0.0f, glm::vec3(0.0f, 0.25f, 0.45f)));
	colorFunc.push_back(std::tuple<GLfloat, glm::vec3>(0.4f, glm::vec3(0.15f, 0.65f, 0.75f)));
	colorFunc.push_back(std::tuple<GLfloat, glm::vec3>(1.0f, glm::vec3(0.15f, 0.65f, 0.75f)));
	colorFunc.push_back(std::tuple<GLfloat, glm::vec3>(1.5f, glm::vec3(1.0f, 1.0f, 1.0f)));

	// Setup the SPHDomain for simulation
	geom3d::Rect bounds = MathHelp::get3dBounds(particlePos.data(), static_cast<UINT>(particlePos.size()));
	sphDomain = new SPHDomain();
	sphDomain->initParticles(particles, bounds.pos - bounds.size() * 0.5f, bounds.pos + bounds.size() * 0.5f + glm::vec3(1.5f, 0.0f, 0.0f));
	updateParticleMapper();
}
SPHInteractor::~SPHInteractor()
{
	delete sphereSource;
	delete particleMapper;
	delete sphDomain;
}

void SPHInteractor::keyDown(int key) { running = !running; }
void SPHInteractor::keyUp(int key) { }

void SPHInteractor::update()
{
	if (!running)
		return;

#ifdef STATS
	auto start = std::chrono::steady_clock::now();
	printf("Frame: %d\n", iter);
	printf("Total Time: %f\n", iter * SUBSTEPS * TIMESTEP);
#endif
	// Do the actual simulation
	for (UINT i = 0; i < SUBSTEPS; i++)
	{
		sphDomain->update(TIMESTEP);
	}
#ifdef STATS
	auto end = std::chrono::steady_clock::now();
	printf("Sim Time: %fs\n", std::chrono::duration<double, std::milli>(end - start).count() / 1000.0);
	printf("Min,Max Pressure: %f, %f\n", sphDomain.minPressure, sphDomain.maxPressure);
	printf("Max Pressure Force Mag: %f\n", sphDomain.maxPressureForce);
	printf("Max Viscous Force Mag: %f\n\n", sphDomain.maxViscousForce);
#endif

	updateParticleMapper();
	iter++;
}

void SPHInteractor::updateParticleMapper()
{
	// Updates the colors of the particles to the velocities
	glm::vec3* colors = reinterpret_cast<glm::vec3*>(particleMapper->getColorData());
	for (UINT i = 0; i < particleMapper->getInstanceCount(); i++)
	{
		colors[i] = MathHelp::lerp(colorFunc, glm::length(sphDomain->particles[i].velocity) / 1.5f);
	}
	particleMapper->update();
}