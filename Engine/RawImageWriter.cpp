#include "RawImageWriter.h"
#include "ImageData.h"
#include <fstream>

template<class T>
static void writeImage(ImageData* imageData, std::string fileName, T)
{
	T* imagePtr = static_cast<T*>(imageData->getData());
	UINT* dim = imageData->getDimensions();
	UINT numComps = imageData->getNumComps();

	std::ofstream file;
	file.open(fileName);

	file << dim[0] << ' ' << dim[1] << ' ' << dim[2] << ' ' << numComps << ' ' << imageData->getScalarType() << '\n';
	for (UINT i = 0; i < dim[0] * dim[1] * dim[2] * numComps; i++)
	{
		file << imagePtr[i] << ' ';
	}
	file.close();
}

void RawImageWriter::update()
{
	if (fileName == "")
	{
		printf("RawImageWriter: No file name specified.\n");
		return;
	}

	switch (imageData->getScalarType())
	{
		TemplateMacro(writeImage(imageData, fileName, static_cast<TT>(0)));
	default:
		printf("RawImageWriter: Not supported image type.\n");
		break;
	}
}