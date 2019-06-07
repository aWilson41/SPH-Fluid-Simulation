#pragma once

class Renderer;

// The mappers job is too map data to visual elements, it does not do the rendering, just prepares it for rendering.
// This could mean reorganization/mapping to a different form to render
// So for example: With openGL this would consistent of getting the data in a renderable format ON the GPU, oftentimes just moving the data
// Then the corresponding painters job is to render it
// Another good example: Both openGL path tracing and rasterization require data to be on the GPU but would require separate painters
// The renderer notifies the mapper what painter it wants to use
class AbstractMapper
{
public:
	virtual void update() { }

	virtual void draw(Renderer* ren) { }

protected:
	bool forceShader = false;
};