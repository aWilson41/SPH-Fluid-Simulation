#include "SPHDomain.h"
#include "Constants.h"
#include "Engine/Geometry3D.h"
#include "Engine/GlyphPolyDataMapper.h"
#include "Engine/ImageData.h"
#include "Engine/ImageMapper.h"
#include "Engine/Material.h"
#include "Engine/PlaneSource.h"
#include "Engine/PNGWriter.h"
#include "Engine/PolyData.h"
#include "Engine/PolyDataMapper.h"
#include "Engine/Renderer.h"
#include "Engine/RenderWindow.h"
#include "Engine/SphereSource.h"
#include "Engine/TrackballCamera.h"
#include "Engine/TrackballCameraInteractor.h"
#include "Particle.h"
#include <chrono>

// Updates particle poly data with scalar values from particles
void updateParticlePoly(SPHDomain* sphDomain, GlyphPolyDataMapper* mapper);

int main(int argc, char *argv[])
{
	// Create the window
	// This has to happen before any gl calls in other objects because 
	// glfw can only make the opengl context when creating the window.
	RenderWindow renWindow;
	renWindow.setWindowName("MPM Simulation");

	// Create the camera for the renderer to use
	TrackballCamera cam;
	cam.initTrackballCamera(1.4f, 1.57f, 30.0f);

	// Create the renderer
	Renderer ren;
	ren.setCamera(&cam);
	ren.addMaterial(Material(glm::vec3(0.2f, 0.4f, 0.2f), 0.5f));
	ren.addMaterial(Material(glm::vec3(0.8f, 0.2f, 0.2f), 1.0f));
	ren.addMaterial(Material(glm::vec3(0.2f, 0.2f, 0.8f), 0.4f));
	renWindow.setRenderer(&ren);

	// Setup the camera interactor (maps user window input to camera)
	TrackballCameraInteractor iren;
	iren.setCamera(&cam);
	renWindow.setInteractor(&iren);

	// Setup a ground plane
	PlaneSource planeSource;
	planeSource.update();

	PolyDataMapper planeMapper;
	planeMapper.setInput(planeSource.getOutput());
	planeMapper.setMaterial(ren.getMaterial(0));
	planeMapper.setModelMatrix(MathHelp::matrixScale(5.0f, 3.0f, 2.0f));
	planeMapper.update();
	ren.addRenderItem(&planeMapper);

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

	SphereSource sphereSource;
	sphereSource.setRadius(h * 0.5f);
	sphereSource.update();
	GlyphPolyDataMapper glyphMapper;
	glyphMapper.setInput(sphereSource.getOutput());
	glyphMapper.allocateOffsets(static_cast<UINT>(particlePos.size()));
	glyphMapper.allocateColorData(static_cast<UINT>(particlePos.size()));
	glm::vec3* offsetData = reinterpret_cast<glm::vec3*>(glyphMapper.getOffsetData());
	std::vector<Particle> particles(particlePos.size());
	for (UINT i = 0; i < particlePos.size(); i++)
	{
		offsetData[i] = particlePos[i];
		particles[i] = Particle(&offsetData[i], PARTICLE_MASS);
	}

	glyphMapper.update();
	ren.addRenderItem(&glyphMapper);

	// Setup the SPHDomain for simulation
	geom3d::Rect bounds = MathHelp::get3dBounds(particlePos.data(), static_cast<UINT>(particlePos.size()));
	SPHDomain sphDomain;
	sphDomain.initParticles(particles, bounds.pos - bounds.size() * 0.5f, bounds.pos + bounds.size() * 0.5f + glm::vec3(1.0f, 0.0f, 0.0f));

	// Update loop
	UINT frameCount = 0;
	while (renWindow.isActive())
	{
#ifdef STATS
		auto start = std::chrono::steady_clock::now();
		printf("Frame: %d\n", frameCount);
		printf("Total Time: %f\n", frameCount * SUBSTEPS * TIMESTEP);
#endif
		// Do the actual simulation
		for (UINT i = 0; i < SUBSTEPS; i++)
		{
			sphDomain.update(TIMESTEP);
		}
#ifdef STATS
		auto end = std::chrono::steady_clock::now();
		printf("Sim Time: %fs\n", std::chrono::duration<double, std::milli>(end - start).count() / 1000.0);
		printf("Min,Max Pressure: %f, %f\n", sphDomain.minPressure, sphDomain.maxPressure);
		printf("Max Pressure Force Mag: %f\n", sphDomain.maxPressureForce);
		printf("Max Viscous Force Mag: %f\n\n", sphDomain.maxViscousForce);
#endif

		updateParticlePoly(&sphDomain, &glyphMapper);
		renWindow.render();
#ifdef OUTPUTFRAMES
		if (frameCount < MAXOUTPUTFRAMES)
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
			writer.setFileName("output/frame_" + std::to_string(frameCount) + ".png");
			writer.setInput(&image);
			writer.update();
		}
#endif
		frameCount++;
	}

	return 1;
}

// Updates color info based on particle velocities
void updateParticlePoly(SPHDomain* sphDomain, GlyphPolyDataMapper* mapper)
{
	glm::vec3* colors = reinterpret_cast<glm::vec3*>(mapper->getColorData());
	for (UINT i = 0; i < mapper->getInstanceCount(); i++)
	{
		//colors[i] = MathHelp::lerp(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.1f, 0.1f, 1.0f), MathHelp::clamp(sphDomain->particles[i].neighbors.size() / 15.0f, 0.0f, 1.0f));
		colors[i] = MathHelp::lerp(glm::vec3(0.1f, 0.1f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), MathHelp::clamp(glm::length(sphDomain->particles[i].velocity) / 1.5f, 0.0f, 1.0f));
		//colors[i] = MathHelp::lerp(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.1f, 0.1f, 1.0f), MathHelp::clamp(glm::length(sphDomain->particles[i].pressure) / 600.0f, 0.0f, 1.0f));
	}
	mapper->update();
}