#pragma once
#include "AttribData.h"
#include "MathHelper.h"

// Defines geometry
class PointData : public AttribData
{
public:
	~PointData()
	{
		if (data != nullptr)
			delete[] data;
	}

public:
	GLfloat* data = nullptr;
	UINT count = 0;
};