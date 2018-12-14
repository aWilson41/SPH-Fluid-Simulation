#include "SPHInteractor.h"
#include "Constants.h"
#include "Engine/Geometry3D.h"
#include "Engine/GlyphPolyDataMapper.h"
#include "Engine/ImageData.h"
#include "Engine/PNGWriter.h"
#include "Engine/SphereSource.h"
#include "SPHDomain.h"
#ifdef STATS
#include <chrono>
#endif
#include <GLFW/glfw3.h>
#include <tuple>

SPHInteractor::SPHInteractor()
{
	// Set the particle positions
	std::vector<glm::vec3> particlePos;
	GLfloat iterLength = h / 1.1f; // Squish the particles together a bit for initialization
	for (GLfloat x = -10.0f; x < 10.0f; x += iterLength)
	{
		for (GLfloat y = -10.0f; y < 10.0f; y += iterLength)
		{
			for (GLfloat z = -10.0f; z < 10.0f; z += iterLength)
			{
				if (x < 0.5f && x > -0.25f && 
					y < 1.5f && y > 0.0f && 
					z < 0.25f && z > -0.25f && x < -0.5f * y + 0.5f)
					particlePos.push_back(glm::vec3(x, y, z));
			}
		}
	}

	// Create a uv sphere source for instancing
	sphereSource = new SphereSource();
	sphereSource->setRadius(h * 0.5f);
	sphereSource->update();
	// Create the particle mapper
	particleMapper = new GlyphPolyDataMapper();
	particleMapper->setInput(sphereSource->getOutput());
	particleMapper->allocateOffsets(static_cast<UINT>(particlePos.size()));
	particleMapper->allocateColorData(static_cast<UINT>(particlePos.size()));
	glm::vec3* offsetData = reinterpret_cast<glm::vec3*>(particleMapper->getOffsetData());
	// Set the offset data of particle mapper with the generate positions and create the particles
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
	bounds.extent *= glm::vec3(1.0f, 1.0f, 1.2f);
	sphDomain = new SPHDomain();
	sphDomain->initParticles(particles, bounds.pos - bounds.size() * 0.5f, bounds.size() * 0.5f + glm::vec3(1.5f, 0.0f, 0.0f));
	updateParticleMapper();
}
SPHInteractor::~SPHInteractor()
{
	delete sphereSource;
	delete particleMapper;
	delete sphDomain;
}

void SPHInteractor::keyDown(int key)
{
	if (key == GLFW_KEY_P)
		writingFrames = !writingFrames;
	else if (key == GLFW_KEY_ENTER)
		running = !running;
}
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

#ifdef OUTPUTFRAMES
	if (writingFrames && iter < MAXOUTPUTFRAMES)
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
	}
#endif
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