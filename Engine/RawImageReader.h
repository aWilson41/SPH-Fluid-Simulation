#pragma once
#include <string>

class ImageData;

class RawImageReader
{
public:
	~RawImageReader();

public:
	ImageData* getOutput() { return imageData; }
	std::string getFileName() { return fileName; }

	void setFileName(std::string fileName) { RawImageReader::fileName = fileName; }

	void update();

protected:
	ImageData* imageData = nullptr;
	std::string fileName = "";
};