#pragma once
#include "MathHelper.h"
#include "Types.h"

// Stores image data of uchar, int, float, or double
class ImageData
{
public:
	ImageData() { }
	~ImageData();

	void allocate2DImage(UINT* dim, double* spacing, double* origin, UINT numComps, ScalarType type);
	void allocate3DImage(UINT* dim, double* spacing, double* origin, UINT numComps, ScalarType type);

	void setSpacing(double x, double y, double z)
	{ 
		spacing[0] = x;
		spacing[1] = y;
		spacing[2] = z;
	}
	void setOrigin(double x, double y, double z)
	{
		origin[0] = x;
		origin[1] = y;
		origin[2] = z;
	}

	UINT* getDimensions() { return dim; }
	double* getBounds() { return bounds; }
	double* getSpacing() { return spacing; }
	double* getOrigin() { return origin; }
	void* getData() { return data; }
	UINT getNumComps() { return numComps; }
	ScalarType getScalarType() { return type; }

	// Updates bounds given dimensions, spacing, & origin
	void updateBounds();

protected:
	template<class T>
	void allocateData(T, UINT count) { data = new T[count]; }

protected:
	void* data = nullptr;
	UINT dim[3] = { 0, 0, 0 };
	UINT numComps = 1;
	ScalarType type = ScalarType::UCHAR_T;

	double spacing[3] = { 1.0, 1.0, 1.0 };
	double origin[3] = { 0.0, 0.0, 0.0 };
	double bounds[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
};