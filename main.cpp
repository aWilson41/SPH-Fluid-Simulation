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

#include <BilateralRBlurPass.h>
#include <ComputeNormalsPass.h>
#include <CubeMapPass.h>
#include <DepthToRPass.h>
#include <GeometryPass.h>
#include <LightingPass.h>

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


	// Create the renderer
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

	// Setup a ground plane
	PlaneSource planeSource;
	planeSource.update();
	PolyDataMapper planeMapper;
	planeMapper.setInput(planeSource.getOutput());
	planeMapper.setMaterial(ren.getMaterial(0));
	planeMapper.setModelMatrix(MathHelp::matrixScale(5.0f, 1.0f, 5.0f));
	planeMapper.update();
	ren.addRenderItem(&planeMapper);


	// Render a cubemap once without fluid
	//CubeMapPass* cubeMapPass = new CubeMapPass();
	//cubeMapPass->setPassDim(128, 128);

	//ren.addPass(cubeMapPass);

	//// Do the initial render
	//renWindow.render();

	//// Remove the pass
	//ren.removePass(cubeMapPass);

	// Add the fluid
	ren.addRenderItem(iren.getParticleMapper());


	// Setup the passes for the main loop
	// Render the geometry into a renderbuffer
	GeometryPass* geomPass = new GeometryPass();

	// Do the lighting pass
	LightingPass* lightPass = new LightingPass();
	lightPass->setPosInput(geomPass->getPosOutput());
	lightPass->setNormalInput(geomPass->getNormalOutput());
	lightPass->setDiffuseInput(geomPass->getDiffuseOutput());
	lightPass->setAmbientInput(geomPass->getAmbientOutput());

	// Write the depth to a single component color buffer and linearize
	DepthToRPass* depthRenderPass = new DepthToRPass();
	depthRenderPass->setDepthInput(geomPass->getDepthOutput());

	// Do an edge preserved blur on the linearized depth image
	BilateralRBlurPass* depthBlurPass = new BilateralRBlurPass();
	depthBlurPass->setColorInput(depthRenderPass->getColorOutput());

	// Reconstruct normals from the blurred depth
	ComputeNormalsPass* depthNormalsPass = new ComputeNormalsPass();
	depthNormalsPass->setColorInput(depthBlurPass->getColorOutput());

	// Re-render with new normals, specularity, fresnel
	LightingPass* lightPass2 = new LightingPass();
	lightPass2->setPosInput(geomPass->getPosOutput());
	lightPass2->setNormalInput(depthNormalsPass->getNormalOutput());
	lightPass2->setDiffuseInput(geomPass->getDiffuseOutput());
	lightPass2->setAmbientInput(geomPass->getAmbientOutput());

	ren.addPass(geomPass);
	//ren.addPass(lightPass);
	ren.addPass(depthRenderPass);
	ren.addPass(depthBlurPass);
	ren.addPass(depthNormalsPass);
	ren.addPass(lightPass2);

	// Update loop
	while (renWindow.isActive())
	{
		iren.update(); // Process simulation and interactor
		renWindow.render(); // Render and poll for input
	}

	return EXIT_SUCCESS;
}