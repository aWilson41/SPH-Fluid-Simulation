#pragma once

// Defines an attribute set of 8
class AttribData
{
public:
	AttribData()
	{
		for (unsigned int i = 0; i < 8; i++)
		{
			attrib[i] = nullptr;
		}
	}
	~AttribData()
	{
		for (unsigned int i = 0; i < 8; i++)
		{
			if (attrib[i] != nullptr)
				delete[] attrib[i];
		}
	}

public:
	void* attrib[8];
};