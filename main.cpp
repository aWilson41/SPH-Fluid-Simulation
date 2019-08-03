#include "Engine/DeferredRenderer.h"
#include "Engine/GlyphPolyDataMapper.h"
#include "Engine/ImageData.h"
#include "Engine/PhongMaterial.h"
#include "Engine/PlaneSource.h"
#include "Engine/PNGWriter.h"
#include "Engine/RayTraceRenderer.h"
#include "Engine/RenderWindow.h"
#include "Engine/TrackballCamera.h"
#include "SPHInteractor.h"

#include "Engine/GeometryPass.h"
#include "Engine/LightingPass.h"
#include "Engine/UnsharpMaskingPass.h"

int main(int argc, char *argv[])
{
	// Create the window
	// This has to happen before any gl calls in other objects because 
	// glfw can only make the opengl context when creating the window.
	RenderWindow renWindow("SPH Simulation");

	// Create the camera for the renderer to use
	TrackballCamera cam;
	cam.initTrackballCamera(0.7f, 1.57f, 30.5f);
	cam.setFocalPt(0.001505f, -0.000141f, 0.014044f);

	// Create the renderer
	DeferredRenderer ren(false);
	//RayTraceRenderer ren;
	//Renderer ren;
	ren.setCamera(&cam);
	ren.addMaterial(PhongMaterial(glm::vec3(0.2f, 0.4f, 0.2f), 0.5f));
	ren.addMaterial(PhongMaterial(glm::vec3(0.2f, 0.2f, 0.2f), 0.5f));
	ren.addMaterial(PhongMaterial(glm::vec3(0.0f, 0.0f, 0.2f), 0.5f));

	// Setup a custom render pass for AO
	GeometryPass* geomPass = new GeometryPass();

	LightingPass* lightPass = new LightingPass();
	lightPass->setPosInput(geomPass->getPosOutput());
	lightPass->setNormalInput(geomPass->getNormalOutput());
	lightPass->setDiffuseInput(geomPass->getDiffuseOutput());
	lightPass->setAmbientInput(geomPass->getAmbientOutput());

	UnsharpMaskingPass* aoPass = new UnsharpMaskingPass();
	aoPass->setColorInput(lightPass->getColorOutput());
	aoPass->setDepthInput(geomPass->getDepthOutput());

	ren.addPass(geomPass);
	ren.addPass(lightPass);
	ren.addPass(aoPass);

	renWindow.setRenderer(&ren);

	// Setup the camera interactor (maps user window input to camera)
	SPHInteractor iren;
	iren.setRenderer(&ren);
	iren.setCamera(&cam);
	renWindow.setInteractor(&iren);
	ren.addRenderItem(iren.getParticleMapper());

	// Setup a ground plane
	/*PlaneSource planeSource;
	planeSource.update();
	PolyDataMapper planeMapper;
	planeMapper.setInput(planeSource.getOutput());
	planeMapper.setMaterial(ren.getMaterial(0));
	planeMapper.setModelMatrix(MathHelp::matrixScale(5.0f, 1.0f, 5.0f));
	planeMapper.update();
	ren.addRenderItem(&planeMapper);*/

	// Update loop
	while (renWindow.isActive())
	{
		// Update the interactor (processes input and the simulation)
		iren.update();
		// Do the render, swap buffers, poll for input
		renWindow.render();
	}

	return EXIT_SUCCESS;
}