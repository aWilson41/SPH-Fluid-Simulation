#include "GLSLSPHDomain.h"
#include "Constants.h"
#include <ShaderProgram.h>
#include <Shaders.h>

static int nextPowerOfTwo(int x) {
	x--;
	x |= x >> 1; // handle 2 bit numbers
	x |= x >> 2; // handle 4 bit numbers
	x |= x >> 4; // handle 8 bit numbers
	x |= x >> 8; // handle 16 bit numbers
	x |= x >> 16; // handle 32 bit numbers
	x++;
	return x;
}

GLSLSPHDomain::GLSLSPHDomain()
{
	// Load the compute shader
	shaderProgram = Shaders::loadComputeShader("main", "Shaders/computeSPH.glsl");
}

void GLSLSPHDomain::initParticles(std::vector<SPHParticle> particles, glm::vec3 origin, glm::vec3 size, GLfloat bufferRatio)
{
	GLSLSPHDomain::particles = particles;
	GLSLSPHDomain::origin = origin;
	GLSLSPHDomain::size = size;
	bounds[0] = origin.x;
	bounds[1] = origin.x + size.x;
	bounds[2] = origin.y;
	bounds[3] = origin.y + size.y;
	bounds[4] = origin.z;
	bounds[5] = origin.z + size.z;

	// Copy the particle data to a gpu format
	gpuParticles.resize(particles.size());
	for (size_t i = 0; i < particles.size(); i++)
	{
		gpuParticles[i].position = particles[i].getPos();
		gpuParticles[i].velocity = particles[i].velocity;
		gpuParticles[i].acceleration = particles[i].accel;
		gpuParticles[i].mass = particles[i].mass;
		gpuParticles[i].density = particles[i].density;
		gpuParticles[i].pressure = particles[i].pressure;
	}

	// Put them on the gpu
	glGenBuffers(1, &particleBufferID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBufferID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUParticle) * gpuParticles.size(), gpuParticles.data(), GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleBufferID); // Point this buffer to binding point 0 in shader
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void GLSLSPHDomain::update(GLfloat dt)
{
	// Bin the particles into local areas
	/*bins = std::vector<std::vector<SPHParticle*>>(gridWidth * gridHeight * gridDepth);
	for (UINT i = 0; i < particles.size(); i++)
	{
		SPHParticle* p = &particles[i];
		p->gridX = MathHelp::clamp(static_cast<int>(gridWidth * (p->pos->x - bufferBounds[0]) / bufferSize.x), 0, gridWidth - 1);
		p->gridY = MathHelp::clamp(static_cast<int>(gridHeight * (p->pos->y - bufferBounds[2]) / bufferSize.y), 0, gridHeight - 1);
		p->gridZ = MathHelp::clamp(static_cast<int>(gridDepth * (p->pos->z - bufferBounds[4]) / bufferSize.z), 0, gridDepth - 1);
		int binIndex = calcIndex(p->gridX, p->gridY, p->gridZ, gridWidth, gridHeight);
		bins[binIndex].push_back(p);
	}*/

	// Compute densities
	invokeComputeShader(0, dt);

	// Compute forces
	invokeComputeShader(1, dt);

	// Integrate + collisions
	invokeComputeShader(2, dt);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBufferID);
	//GPUParticle* test = static_cast<GPUParticle*>(glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY));
	//// Copy the resulting positions for the user
	//for (size_t i = 0; i < particles.size(); i++)
	//{
	//	(*particles[i].pos) = test[i].position;
	//}
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBufferID);
	GPUParticle* test = new GPUParticle[particles.size()];
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GPUParticle) * particles.size(), test);
	// Copy the resulting positions for the user
	for (size_t i = 0; i < particles.size(); i++)
	{
		(*particles[i].pos) = test[i].position;
		particles[i].velocity = test[i].velocity;
	}
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	delete[] test;
}

void GLSLSPHDomain::invokeComputeShader(unsigned int taskID, GLfloat dt)
{
	// Bind the shader
	GLuint shaderID = shaderProgram->getProgramID();
	glUseProgram(shaderID);

	// Binds uniforms
	glUniform1f(0, dt);
	glUniform1f(1, GAMMA);
	glUniform1f(2, KAPPA);
	glUniform1f(3, REST_DENSITY);
	glUniform1f(4, FRICTION);
	glUniform1f(5, VISCOSITY);
	glUniform1f(6, h);
	glUniform1f(7, h2);
	glUniform1f(8, poly6Coe);
	glUniform1f(9, spikyGradCoe);
	glUniform1f(10, polyLapCoe);
	glUniform1fv(11, 6, bounds);
	glUniform1ui(17, taskID);

	// Bind the buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBufferID);

	// Launch the shader
	static GLint workGroupSize[3] = { -1, -1, -1 };
	glGetProgramiv(shaderID, GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);
	int numGroupsX = nextPowerOfTwo(particles.size()) / workGroupSize[0];
	glDispatchCompute(numGroupsX, 1, 1);

	// Block until complete
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glUseProgram(0);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}