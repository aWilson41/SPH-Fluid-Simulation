#pragma once
#include "MathHelper.h"
#include <string>

class DeferredRenderer;

class RenderPass
{
public:
	RenderPass(std::string name) { passName = name; }

public:
	void setPassDim(int width, int height)
	{
		fboWidth = width;
		fboHeight = height;
	}
	void setInput(size_t portNum, GLuint* glId) { inputs[portNum] = glId; }
	void setNumberOfInputPorts(GLuint numberOfPorts)
	{
		inputs.resize(numberOfPorts);
		std::fill_n(inputs.data(), inputs.size(), nullptr);
	}
	void setNumberOfOutputPorts(GLuint numberOfPorts) { outputs.resize(numberOfPorts); }

	GLuint getNumberOfInputPorts() { return static_cast<GLuint>(inputs.size()); }
	GLuint getNumberOfOutputPorts() { return static_cast<GLuint>(outputs.size()); }
	GLuint* getInput(size_t portNum) { return inputs[portNum]; };
	GLuint getOutput(size_t portNum) { return outputs[portNum]; }
	std::vector<GLuint*> getInputs() { return inputs; }
	std::vector<GLuint> getOutputs() { return outputs; }
	std::string getPassName() { return passName; }
	GLuint getFramebuffer() { return fboID; }

public:
	virtual void render(DeferredRenderer* ren) = 0;

	virtual void resizeFramebuffer(int width, int height) = 0;

protected:
	GLuint fboID = -1;
	std::string passName = "Unnamed";
	std::vector<GLuint*> inputs;
	std::vector<GLuint> outputs;
	int fboWidth = 100;
	int fboHeight = 100;
};