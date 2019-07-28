#include "PNGWriter.h"
#include "ImageData.h"
#include "LodePNG/lodepng.h"

void PNGWriter::update()
{
	if (fileName == "")
	{
		printf("PNGWriter: No file name specified.\n");
		return;
	}

	if (imageData->getScalarType() != UCHAR_T)
	{
		printf("PNGWriter: Can only write unsigned char images.\n");
		return;
	}

	// Copy the image to std vector because that's how lodepng wants it (add component)
	UINT* dim = imageData->getDimensions();
	std::vector<unsigned char> image(dim[0] * dim[1] * 4);
	unsigned char* data = static_cast<unsigned char*>(imageData->getData());

	UINT numComps = imageData->getNumComps();
	if (numComps == 1)
	{
		for (UINT i = 0; i < dim[0] * dim[1]; i++)
		{
			UINT i2 = i * 4;
			image[i2] = data[i];
			image[i2 + 1] = data[i];
			image[i2 + 2] = data[i];
			image[i2 + 3] = 255;
		}
	}
	else if (numComps == 3)
	{
		for (UINT i = 0; i < dim[0] * dim[1]; i++)
		{
			UINT i1 = i * 3;
			UINT i2 = i * 4;
			image[i2] = data[i1];
			image[i2 + 1] = data[i1 + 1];
			image[i2 + 2] = data[i1 + 2];
			image[i2 + 3] = 255;
		}
	}
	else if (numComps == 4)
	{
		for (UINT i = 0; i < dim[0] * dim[1] * 4; i++)
		{
			image[i] = data[i];
		}
	}
	else
	{
		printf("PNGWriter: Can only write 1, 3, or 4 component images.\n");
		return;
	}

	// Write the image with lodepng
	if (lodepng::encode(fileName, image, dim[0], dim[1]))
		printf("PNGWriter: LodePNG Error\n");
}