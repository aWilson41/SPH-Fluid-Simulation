#include "SPHRasterizer.h"
#include "SPHDomain.h"

static GLfloat NX(GLfloat x)
{
	x = fabs(x);
	if (x < 1.0f)
		return x * x * (x * 0.5f - 1.0f) + 2.0f / 3.0f;
	else if (x < 2.0f)
		return x * (x * (-x / 6.0f + 1.0f) - 2.0f) + 4.0f / 3.0f;
	else
		return 0.0f;
}

static int calcIndex(int x, int y, int z, int width, int height) { return x + (y + height * z) * width; }

static GLfloat linearWeight(glm::vec3 diff) { return glm::length(diff); }
static GLfloat bSplineWeight(glm::vec3 diff) { return NX(diff.x) * NX(diff.y) * NX(diff.z); }

// Returns the domain rasterized onto an image contained within the bounds
ImageData SPHRasterizer::rasterize(UINT* dim, double* spacing)
{
	// Allocate the image
	GLfloat* bounds = sphDomain->bounds;
	double origin[3] = { bounds[0], bounds[2], bounds[4] };
	ImageData data;
	data.allocate2DImage(dim, spacing, origin, 1, ScalarType::FLOAT_T);

	// Fill the image
	if (type == LINEAR)
		raster(&data, 2, &linearWeight);
	else if (type == BSPLINE)
		raster(&data, 2, &bSplineWeight);

	return data;
}

void SPHRasterizer::raster(ImageData* imageData, int r, weightFunction func)
{
	double* spacing = imageData->getSpacing();
	glm::vec3 nodeSize = glm::vec3(spacing[0], spacing[1], spacing[2]);
	glm::vec3 invNodeSize = 1.0f / nodeSize;
	float invNodeVolume = 1.0f / (spacing[0] * spacing[1] * spacing[2]);

	double* org = imageData->getOrigin();
	glm::vec3 origin = glm::vec3(org[0], org[1], org[2]);

	UINT* dim = imageData->getDimensions();
	float* dataPtr = static_cast<float*>(imageData->getData());

	for (UINT i = 0; i < sphDomain->particles.size(); i++)
	{
		Particle& p = sphDomain->particles[i];
		glm::vec3 pPos = p.getPos();
		int gridPosX = static_cast<int>((pPos.x - origin.x) * invNodeSize.x);
		int gridPosY = static_cast<int>((pPos.y - origin.y) * invNodeSize.y);
		int gridPosZ = static_cast<int>((pPos.z - origin.z) * invNodeSize.z);

		int startX = static_cast<UINT>(MathHelp::clamp(gridPosX - r, 0, static_cast<int>(dim[0])));
		int endX = static_cast<UINT>(MathHelp::clamp(gridPosX + r + 1, 0, static_cast<int>(dim[0])));
		int startY = static_cast<UINT>(MathHelp::clamp(gridPosY - r, 0, static_cast<int>(dim[1])));
		int endY = static_cast<UINT>(MathHelp::clamp(gridPosY + r + 1, 0, static_cast<int>(dim[1])));
		int startZ = static_cast<UINT>(MathHelp::clamp(gridPosZ - r, 0, static_cast<int>(dim[2])));
		int endZ = static_cast<UINT>(MathHelp::clamp(gridPosZ + r + 1, 0, static_cast<int>(dim[2])));
		for (UINT z = startZ; z < endZ; z++)
		{
			for (UINT y = startY; y < endY; y++)
			{
				for (UINT x = startX; x < endX; x++)
				{
					glm::vec3 nodePos = glm::vec3(x, y, z) * nodeSize + nodeSize * 0.5f + origin;
					glm::vec3 diff = (p.getPos() - nodePos) * invNodeSize;
					dataPtr[calcIndex(x, y, z, dim[0], dim[1])] += p.mass * func(diff);
				}
			}
		}
	}
}