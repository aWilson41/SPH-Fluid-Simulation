#pragma once
#include <string>

class ImageData;

class RawImageWriter
{
public:
	void setInput(ImageData* data) { imageData = data; }
	std::string getFileName() { return fileName; }

	void setFileName(std::string fileName) { RawImageWriter::fileName = fileName; }

	void update();

protected:
	ImageData* imageData = nullptr;
	std::string fileName = "";
};