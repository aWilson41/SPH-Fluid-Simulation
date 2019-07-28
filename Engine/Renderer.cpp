#include "Renderer.h"
#include "AbstractMapper.h"
#include "ImageData.h"
#include "PhongMaterial.h"
#include "Shaders.h"

Renderer::Renderer()
{
	glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
}

Renderer::~Renderer()
{
	Shaders::deleteShaders();

	for (UINT i = 0; i < materials.size(); i++)
	{
		delete materials[i];
	}
}

void Renderer::addMaterial(PhongMaterial material) { materials.push_back(new PhongMaterial(material)); }

bool Renderer::containsRenderItem(AbstractMapper* mapper)
{
	for (UINT i = 0; i < mappers.size(); i++)
	{
		if (mappers[i] == mapper)
			return true;
	}
	return false;
}

void Renderer::render()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (UINT i = 0; i < mappers.size(); i++)
	{
		AbstractMapper* mapper = mappers[i];
		mapper->use(this);

		// Set the scene uniforms
		GLuint lightDirLocation = glGetUniformLocation(mapper->getShaderProgramID(), "lightDir");
		if (lightDirLocation != -1)
			glUniform3fv(lightDirLocation, 1, &lightDir[0]);

		mapper->draw(this);
	}
}

void Renderer::resizeFramebuffer(int width, int height)
{
	framebufferWidth = width;
	framebufferHeight = height;
	glViewport(0, 0, width, height);
}

ImageData* Renderer::getOutputImage()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	ImageData* results = new ImageData();
	UINT dim[2] = { framebufferWidth, framebufferHeight };
	double spacing[2] = { 1.0, 1.0 };
	double origin[2] = { 0.0, 0.0 };
	results->allocate2DImage(dim, spacing, origin, 4, ScalarType::UCHAR_T);

	unsigned char* buffer = new unsigned char[dim[0] * dim[1] * 4];
	glReadPixels(0, 0, framebufferWidth, framebufferHeight, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	unsigned char* imgPtr = static_cast<unsigned char*>(results->getData());
	// Flip the image
	for (UINT y = 0; y < dim[1]; y++)
	{
		for (UINT x = 0; x < dim[0]; x++)
		{
			UINT index1 = (y * framebufferWidth + x) * 4;
			UINT index2 = (((framebufferHeight - 1) - y) * framebufferWidth + x) * 4;
			imgPtr[index1] = buffer[index2];
			imgPtr[index1 + 1] = buffer[index2 + 1];
			imgPtr[index1 + 2] = buffer[index2 + 2];
			imgPtr[index1 + 3] = buffer[index2 + 3];
		}
	}
	delete[] buffer;
	return results;
}

void Renderer::setClearColor(float r, float g, float b, float a)
{
	clearColor[0] = r;
	clearColor[1] = g;
	clearColor[2] = b;
	clearColor[3] = a;
	glClearColor(r, g, b, a);
}