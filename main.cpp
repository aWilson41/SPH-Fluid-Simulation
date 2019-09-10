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
#include "Engine/DepthToRPass.h"
#include "Engine/ComputeNormalsPass.h"
#include "Engine/BilateralRBlurPass.h"

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

	// Setup a custom render pass for AO
	// Render the geometry into a renderbuffer
	GeometryPass* geomPass = new GeometryPass();

	// Do the lighting pass
	/*LightingPass* lightPass = new LightingPass();
	lightPass->setPosInput(geomPass->getPosOutput());
	lightPass->setNormalInput(geomPass->getNormalOutput());
	lightPass->setDiffuseInput(geomPass->getDiffuseOutput());
	lightPass->setAmbientInput(geomPass->getAmbientOutput());*/

	// Write the depth to a single component color buffer and linearize
	DepthToRPass* depthRenderPass = new DepthToRPass();
	depthRenderPass->setDepthInput(geomPass->getDepthOutput());

	BilateralRBlurPass* depthBlurPass = new BilateralRBlurPass();
	depthBlurPass->setColorInput(depthRenderPass->getColorOutput());

	// Reconstruct normals from the blurred depth
	ComputeNormalsPass* depthNormalsPass = new ComputeNormalsPass();
	depthNormalsPass->setColorInput(depthBlurPass->getColorOutput());

	// Re-render with new normals (inefficient will be replaced soon)
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

	renWindow.setRenderer(&ren);

	// Setup the camera interactor (maps user window input to camera)
	SPHInteractor iren;
	iren.setRenderer(&ren);
	iren.setCamera(&cam);
	renWindow.setInteractor(&iren);
	ren.addRenderItem(iren.getParticleMapper());

	// Setup a ground plane
	PlaneSource planeSource;
	planeSource.update();
	PolyDataMapper planeMapper;
	planeMapper.setInput(planeSource.getOutput());
	planeMapper.setMaterial(ren.getMaterial(0));
	planeMapper.setModelMatrix(MathHelp::matrixScale(5.0f, 1.0f, 5.0f));
	planeMapper.update();
	ren.addRenderItem(&planeMapper);

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