#pragma once
#include <ImageData.h>

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
	~SPHRasterizer();

public:
	void setSPHDomain(SPHDomain* domain) { sphDomain = domain; }
	void setDim(UINT dimX, UINT dimY, UINT dimZ)
	{
		dim[0] = dimX;
		dim[1] = dimY;
		dim[2] = dimZ;
	}
	void setSize(glm::vec3 size) { SPHRasterizer::size = size; }

	ImageData* getOutput() { return imageData; }

	void update();

protected:
	void raster(ImageData* imageData, int r, weightFunction func);

protected:
	SPHDomain* sphDomain = nullptr;
	ImageData* imageData = nullptr;
	UINT dim[3] = { 0, 0, 0 };
	glm::vec3 size;
	FuncType type = BSPLINE;
};