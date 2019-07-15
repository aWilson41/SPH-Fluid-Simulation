#pragma once

class Renderer;

// The mappers job is too map data to visual elements, it does not do the rendering, just prepares it for rendering.
// For example one renderer might prefer polygons with indices while another is more strict.
// So it just does reorganization/mapping to a different form to render.
// All mappers assume the use of OpenGL
class AbstractMapper
{
public:
	virtual void update() { }

	virtual void draw(Renderer* ren) { }

protected:
	bool forceShader = false;
};