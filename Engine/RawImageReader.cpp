#include "RawImageReader.h"
#include "ImageData.h"
#include <fstream>

template<class T>
static void readImage(std::ifstream* file, ImageData* imageData, T)
{
	T* imagePtr = static_cast<T*>(imageData->getData());
	UINT* dim = imageData->getDimensions();
	UINT numComps = imageData->getNumComps();

	// Read in the image
	for (UINT i = 0; i < dim[0] * dim[1] * dim[2] * numComps; i++)
	{
		(*file) >> imagePtr[i];
	}
}

RawImageReader::~RawImageReader()
{
	if (imageData != nullptr)
		delete imageData;
}

void RawImageReader::update()
{
	if (fileName == "")
	{
		printf("RawImageReader: No file name specified.\n");
		return;
	}

	std::ifstream file;
	file.open(fileName);
	if (file.fail())
	{
		printf(("Failed to open file " + fileName + '\n').c_str());
		return;
	}

	imageData = new ImageData();
	static UINT dim[3] = { 0, 0, 0 };
	static double spacing[3] = { 1.0, 1.0, 1.0 };
	static double origin[3] = { 0.0, 0.0, 0.0 };
	UINT numComps = 0;
	int type = 0;
	file >> dim[0] >> dim[1] >> dim[2] >> numComps >> type;
	// Allocate an image
	if (dim[2] == 1)
		imageData->allocate2DImage(dim, spacing, origin, numComps, static_cast<ScalarType>(type));
	else
		imageData->allocate3DImage(dim, spacing, origin, numComps, static_cast<ScalarType>(type));

	switch (imageData->getScalarType())
	{
		TemplateMacro(readImage(&file, imageData, static_cast<TT>(0)));
	default:
		printf("RawImageReader: Not supported image type.\n");
		break;
	}
	file.close();
}