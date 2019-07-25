#pragma once
#include "PolyDataMapper.h"
#include "Types.h"

// Does rendering and pre rendering operations (mapping input to visual representation)
// Generalized class, makes no assumptions about data provided too it at the cost of some optimization
// This class does instanced drawing with the input and offset data provided
// Currently only supports normals
class GlyphPolyDataMapper : public PolyDataMapper
{
public:
	GlyphPolyDataMapper();
	~GlyphPolyDataMapper();

public:
	//void setInput(PolyData* input);

	void allocateOffsets(UINT count)
	{
		instanceCount = count;
		if (offsetData != nullptr)
			delete[] offsetData;
		offsetData = new GLfloat[count * 3];
	}
	void allocateColorData(UINT count)
	{
		instanceCount = count;
		if (colorData != nullptr)
			delete[] colorData;
		colorData = new GLfloat[count * 3];
	}

	GLfloat* getOffsetData() { return offsetData; }
	GLfloat* getColorData() { return colorData; }
	UINT getInstanceCount() { return instanceCount; }

	// Updates the buffer based on the set poly data. If it's the first time it initializes the buffer
	void update() override;

	void use(Renderer* ren) override;

	void draw(Renderer* ren) override;

	GLuint getShaderProgramID() override;

protected:
	void updateInfo();
	void updateBuffer();

protected:
	GLuint instanceCount = 0;
	GLfloat* offsetData = nullptr;
	GLfloat* colorData = nullptr;
};