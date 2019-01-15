#include "RawImageWriter.h"
#include "ImageData.h"
#include <fstream>

template<class T>
static void writeImage(ImageData* imageData, std::string fileName, T)
{
	T* imagePtr = static_cast<T*>(imageData->getData());
	UINT* dim = imageData->getDimensions();

	std::ofstream file;
	file.open(fileName);

	file << dim[0] << " " << dim[1] << " " << dim[2] << std::endl;
	UINT numComps = imageData->getNumComps();
	UINT i = 0;
	for (UINT z = 0; z < dim[2]; z++)
	{
		for (UINT y = 0; y < dim[1]; y++)
		{
			for (UINT x = 0; x < dim[0]; x++)
			{
				for (UINT j = 0; j < numComps; j++)
				{
					file << static_cast<float>(imagePtr[i]) << " ";
					i++;
				}
			}
		}
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