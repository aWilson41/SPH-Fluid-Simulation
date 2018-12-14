#include "SphereSource.h"
#include "PolyData.h"

SphereSource::SphereSource() { outputData = new PolyData(); }
SphereSource::~SphereSource()
{
	if (outputData != nullptr)
		delete outputData;
}
void SphereSource::update()
{
	if (outputData == nullptr)
		return;

	// Vertices
	UINT vertexCount = (divTheta - 1) * divPhi + 2;
	outputData->allocateSharedVertexData(vertexCount, TRIANGLE);
	glm::vec3* vData = reinterpret_cast<glm::vec3*>(outputData->getVertexData());
	vData[0] = glm::vec3(0.0f, radius, 0.0f);

	UINT iter = 1;
	for (UINT j = 0; j < divTheta - 1; j++)
	{
		GLfloat polar = PI * static_cast<GLfloat>(j + 1) / divTheta;
		GLfloat sp = std::sin(polar);
		GLfloat cp = std::cos(polar);
		for (UINT i = 0; i < divPhi; i++)
		{
			GLfloat azimuth = 2.0f * PI * static_cast<GLfloat>(i) / divPhi;
			GLfloat sa = std::sin(azimuth);
			GLfloat ca = std::cos(azimuth);
			GLfloat x = sp * ca;
			GLfloat y = cp;
			GLfloat z = sp * sa;
			vData[iter++] = glm::vec3(x, y, z) * radius;
		}
	}
	vData[iter] = glm::vec3(0.0f, -radius, 0.0f);

	// Normals
	outputData->allocateNormalData();
	glm::vec3* nData = reinterpret_cast<glm::vec3*>(outputData->getNormalData());
	for (UINT i = 0; i < vertexCount; i++)
	{
		nData[i] = glm::normalize(vData[i]);
	}

	// Indices
	outputData->allocateIndexData(divPhi * 6 * (divTheta - 1));
	GLuint* iData = outputData->getIndexData();

	iter = 0;
	for (UINT i = 0; i < divPhi; i++)
	{
		GLuint a = i + 1;
		GLuint b = (i + 1) % divPhi + 1;
		iData[iter++] = 0;
		iData[iter++] = b;
		iData[iter++] = a;
	}
	for (UINT j = 0; j < divTheta - 2; j++)
	{
		GLuint aStart = j * divPhi + 1;
		GLuint bStart = (j + 1) * divPhi + 1;
		for (UINT i = 0; i < divPhi; i++)
		{
			GLuint a = aStart + i;
			GLuint a1 = aStart + (i + 1) % divPhi;
			GLuint b = bStart + i;
			GLuint b1 = bStart + (i + 1) % divPhi;
			iData[iter++] = a;
			iData[iter++] = a1;
			iData[iter++] = b1;

			iData[iter++] = a;
			iData[iter++] = b1;
			iData[iter++] = b;
		}
	}
	for (UINT i = 0; i < divPhi; i++)
	{
		GLuint a = i + divPhi * (divTheta - 2) + 1;
		GLuint b = (i + 1) % divPhi + divPhi * (divTheta - 2) + 1;
		iData[iter++] = vertexCount - 1;
		iData[iter++] = a;
		iData[iter++] = b;
	}
}