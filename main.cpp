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
#include "Particle.h"
#include "SPHInteractor.h"

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
	SPHInteractor iren;
	iren.setCamera(&cam);
	renWindow.setInteractor(&iren);
	ren.addRenderItem(iren.getParticleMapper());

	// Setup a ground plane
	PlaneSource planeSource;
	planeSource.update();
	PolyDataMapper planeMapper;
	planeMapper.setInput(planeSource.getOutput());
	planeMapper.setMaterial(ren.getMaterial(0));
	planeMapper.setModelMatrix(MathHelp::matrixScale(5.0f, 3.0f, 2.0f));
	planeMapper.update();
	ren.addRenderItem(&planeMapper);

	// Update loop
	while (renWindow.isActive())
	{
		iren.update(); // Updates the simulation

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
	}

	return 1;
}