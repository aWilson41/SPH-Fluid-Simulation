#include "SPHInteractor.h"
#include <DeferredRenderer.h>
#include <GlyphPolyDataMapper.h>
#include <ImageData.h>
#include <PhongMaterial.h>
#include <PlaneSource.h>
#include <PNGWriter.h>
#include <RayTraceRenderer.h>
#include <RenderWindow.h>
#include <TrackballCamera.h>

#include <UnsharpMaskingPass.h>
#include <GeometryPass.h>
#include <LightingPass.h>
#include <DepthToRPass.h>

int main(int argc, char *argv[])
{
	// Create the window
	// This has to happen before any gl calls in other objects because 
	// glfw can only make the opengl context when creating the window.
	RenderWindow renWindow("SPH Simulation");

	// Create the camera for the renderer to use
	TrackballCamera cam;
	cam.initTrackballCamera(0.7f, 1.57f, 30.5f, 70.0f, 0.01f, 10000.0f);
	cam.setFocalPt(0.001505f, -0.000141f, 0.014044f);


	// Create the renderer with default passes
	DeferredRenderer ren(false);
	//RayTraceRenderer ren;
	//Renderer ren;
	ren.setCamera(&cam);
	ren.addMaterial(PhongMaterial(glm::vec3(0.2f, 0.4f, 0.2f), 0.5f));
	ren.addMaterial(PhongMaterial(glm::vec3(0.2f, 0.2f, 0.2f), 0.5f));
	ren.addMaterial(PhongMaterial(glm::vec3(0.0f, 0.0f, 0.2f), 0.5f));
	renWindow.setRenderer(&ren);

	// Setup the camera interactor (maps user window input to camera)
	SPHInteractor iren;
	iren.setRenderer(&ren);
	iren.setCamera(&cam);
	renWindow.setInteractor(&iren);

	// Setup the render passes manually so we can insert an unsharp masking for AO
	GeometryPass* geomPass = new GeometryPass();

	LightingPass* lightPass = new LightingPass();
	lightPass->setPosInput(geomPass->getPosOutput());
	lightPass->setNormalInput(geomPass->getNormalOutput());
	lightPass->setDiffuseInput(geomPass->getDiffuseOutput());
	lightPass->setAmbientInput(geomPass->getAmbientOutput());

	// Linearize depth and put into r color buffer
	DepthToRPass* depthPass = new DepthToRPass();
	depthPass->setDepthInput(geomPass->getDepthOutput());

	UnsharpMaskingPass* aoPass = new UnsharpMaskingPass();
	aoPass->setRadiusRatio(0.0025f);
	aoPass->setSigma(5.0f);
	aoPass->setDarknessFactor(30.0f);
	aoPass->setColorInput(lightPass->getColorOutput());
	aoPass->setDepthInput(depthPass->getColorOutput());
	
	ren.addPass(geomPass);
	ren.addPass(lightPass);
	ren.addPass(depthPass);
	ren.addPass(aoPass);

	// Setup a ground plane
	PlaneSource planeSource;
	planeSource.update();
	PolyDataMapper planeMapper;
	planeMapper.setInput(planeSource.getOutput());
	planeMapper.setMaterial(ren.getMaterial(0));
	planeMapper.setModelMatrix(MathHelp::scale(5.0f, 1.0f, 5.0f));
	planeMapper.update();
	ren.addRenderItem(&planeMapper);

	// Add the fluid
	ren.addRenderItem(iren.getParticleMapper());

	// Update loop
	while (renWindow.isActive())
	{
		iren.update(); // Process simulation and interactor
		renWindow.render(); // Render and poll for input
	}

	return EXIT_SUCCESS;
}