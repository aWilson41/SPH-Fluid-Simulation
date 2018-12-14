#pragma once
#include "AttribData.h"
#include "MathHelper.h"
#include "Types.h"

// Defines topology
class CellData : public AttribData
{
public:
	~CellData()
	{
		if (data != nullptr)
			delete[] data;
	}

public:
	GLuint* data = nullptr;
	UINT cellCount = 0;
	UINT indexCount = 0;
	CellType type = TRIANGLE;
};