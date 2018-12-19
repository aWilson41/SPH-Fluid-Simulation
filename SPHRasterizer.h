#pragma once
#include "Engine/ImageData.h"

class SPHDomain;

typedef GLfloat(*weightFunction)(glm::vec3);

// Rasterizes sph particles to an image, might create a particle collection class to disconnect sphdomain from particles
// since particles can be rasterized whether they are or aren't part of the sph domain
class SPHRasterizer
{
public:
	enum FuncType
	{
		LINEAR = 0,
		BSPLINE = 1
	};

public:
	void setSPHDomain(SPHDomain* domain) { sphDomain = domain; }

	// Returns the domain rasterized onto an image contained within the bounds
	ImageData rasterize(UINT* dim, double* spacing);

protected:
	void raster(ImageData* imageData, int r, weightFunction func);

protected:
	SPHDomain* sphDomain;
	FuncType type = BSPLINE;
};