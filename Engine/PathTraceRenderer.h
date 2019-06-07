#pragma once

class PathTraceRenderer
{
public:
	int getNumberOfSamples() { return numSamples; }
	void setNumberOfSamples(int numSamples) { PathTraceRenderer::numSamples = numSamples; }

protected:
	int numSamples = 4;
};