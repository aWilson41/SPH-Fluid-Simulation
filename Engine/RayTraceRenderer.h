#pragma once
#include "Renderer.h"
#include <chrono>

class ShaderProgram;

// Implements a simple bidirectional ray tracer
class RayTraceRenderer : public Renderer
{
public:
	RayTraceRenderer();

public:
	void render() override;

	void resizeFramebuffer(int width, int height) override;

	std::string getShaderDirectory() override { return "Raytrace/"; }

protected:
	void bindCameraUniforms(GLuint shaderID);
	void bindSceneUniforms(GLuint shaderID);
	void bindRenderUniforms(GLuint shaderID);

private:
	std::chrono::time_point<std::chrono::system_clock> start;

	glm::vec3 currEyeRayDir = glm::vec3(0.0f);
	glm::vec3 prevEyeRayDir = glm::vec3(0.0f);
	GLuint frameCount = 0;

	ShaderProgram* shaderProgram = nullptr;
	ShaderProgram* quadShaderProgram = nullptr;
	GLuint emptyVaoID = -1;
	GLuint colorTexID = -1;
	// GLuint depthBufferID = -1;
};