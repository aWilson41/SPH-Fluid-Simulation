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

static int calcIndex(int x, int y, int z, int width, int height) { return x + width * (y + height * z); }

static int calcIndex(int x, int y, int z, int w, int width, int height, int depth) { return w + width * x + width * height * y + width * height * depth * z; }

GLSLSPHDomain::GLSLSPHDomain()
{
	// Load the compute shader
	computePressureProgram = Shaders::loadComputeShader("main", "Shaders/sphComputePressure.glsl");
	computeForcesProgram = Shaders::loadComputeShader("main", "Shaders/sphComputeForce.glsl");
	integrateProgram = Shaders::loadComputeShader("main", "Shaders/sphIntegrate.glsl");
}

void GLSLSPHDomain::initParticles(std::vector<SPHParticle> particles, glm::vec3 origin, glm::vec3 size, GLfloat bufferRatio)
{
	/*GLint major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	if (major * 10 + minor < 43)
	{
		printf("OpenGL version 4.3 or higher required for this program.\n");
		return;
	}*/

	GLSLSPHDomain::particles = particles;
	GLSLSPHDomain::origin = origin;
	GLSLSPHDomain::size = size;
	bounds[0] = origin.x;
	bounds[1] = origin.x + size.x;
	bounds[2] = origin.y;
	bounds[3] = origin.y + size.y;
	bounds[4] = origin.z;
	bounds[5] = origin.z + size.z;

	glm::vec3 buffer = size * bufferRatio;
	bufferBounds[0] = bounds[0] - buffer[0];
	bufferBounds[1] = bounds[1] + buffer[0];
	bufferBounds[2] = bounds[2] - buffer[1];
	bufferBounds[3] = bounds[3] + buffer[1];
	bufferBounds[4] = bounds[4] - buffer[2];
	bufferBounds[5] = bounds[5] + buffer[2];
	bufferSize = size + buffer;

	glm::vec3 spacing = bufferSize / h;
	gridWidth = static_cast<int>(spacing.x);
	gridHeight = static_cast<int>(spacing.y);
	gridDepth = static_cast<int>(spacing.z);

	// Copy the particle data to a gpu format
	gpuParticles = new GPUParticle[particles.size()];
	for (size_t i = 0; i < particles.size(); i++)
	{
		gpuParticles[i].position = particles[i].getPos();
		gpuParticles[i].velocity = particles[i].velocity;
		gpuParticles[i].acceleration = particles[i].accel;
		gpuParticles[i].mass = particles[i].mass;
		gpuParticles[i].density = particles[i].density;
		gpuParticles[i].pressure = particles[i].pressure;
	}

	// We assume 1/9th particles per bin (first int used for counter)
	/*bins.resize((particles.size() / 9) * (gridWidth * gridHeight * gridDepth));
	std::fill_n(bins.data(), bins.size(), -1);
	for (unsigned int i = 0; i < bins.size(); i += gridWidth * gridHeight * gridDepth)
	{
		bins[i] = 0;
	}*/

	// Put them on the gpu
	glGenBuffers(1, &particleBufferID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBufferID);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUParticle) * particles.size(), gpuParticles, GL_STATIC_DRAW);
	glNamedBufferStorage(particleBufferID, sizeof(GPUParticle) * particles.size(), gpuParticles, GL_DYNAMIC_STORAGE_BIT);
	//glNamedBufferStorage(velBuffer, velocities.size() * sizeof(fhl::Vec4f), velocities.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleBufferID); // Point this buffer to binding point 0 in shader

	//glGenBuffers(1, &binBufferID);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, binBufferID);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * bins.size(), bins.data(), GL_DYNAMIC_DRAW);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, binBufferID); // Point this buffer to binding point 0 in shader

	glGenBuffers(1, &neighborBufferID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, neighborBufferID);
	// Every particle can have at max (1/9th * NumParticles) neighbors
	maxNeighbors = particles.size() / 20;
	//glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * (maxNeighbors * particles.size()), NULL, GL_STATIC_DRAW);
	glNamedBufferStorage(neighborBufferID, sizeof(GLuint) * maxNeighbors * particles.size(), NULL, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, neighborBufferID); // Point this buffer to binding point 1 in shader

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void GLSLSPHDomain::update(GLfloat dt)
{
	// Bin the particles into local areas
	/*std::fill_n(bins.data(), bins.size(), -1);
	for (unsigned int i = 0; i < bins.size(); i += gridWidth * gridHeight * gridDepth)
	{
		bins[i] = 0;
	}
	for (UINT i = 0; i < particles.size(); i++)
	{
		SPHParticle* p = &particles[i];
		p->gridX = MathHelp::clamp(static_cast<int>(gridWidth * (p->pos->x - bufferBounds[0]) / bufferSize.x), 0, gridWidth - 1);
		p->gridY = MathHelp::clamp(static_cast<int>(gridHeight * (p->pos->y - bufferBounds[2]) / bufferSize.y), 0, gridHeight - 1);
		p->gridZ = MathHelp::clamp(static_cast<int>(gridDepth * (p->pos->z - bufferBounds[4]) / bufferSize.z), 0, gridDepth - 1);

		int sizeIndex = calcIndex(p->gridX, p->gridY, p->gridZ, 0, gridWidth, gridHeight, gridDepth);
		GLuint& size = bins[sizeIndex];
		int index = sizeIndex + size;
		bins[index] = i;
		size++;
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, binBufferID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * bins.size(), bins.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);*/


	// Bind the buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBufferID);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, neighborBufferID);

	// Compute densities and neighborhoods
	invokePressureProgram();

	// Possibly order by neighborhood before doing the next step

	// Compute forces
	invokeForcesProgram();

	// Integrate + collisions
	invokeIntegrateProgram(dt);

	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GPUParticle) * particles.size(), gpuParticles);
	// Copy the resulting positions for the user
	for (size_t i = 0; i < particles.size(); i++)
	{
		(*particles[i].pos) = gpuParticles[i].position;
		particles[i].velocity = gpuParticles[i].velocity;
	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void GLSLSPHDomain::invokePressureProgram()
{
	// Bind the shader
	GLuint shaderID = computePressureProgram->getProgramID();
	glUseProgram(shaderID);

	// Binds uniforms
	glUniform1f(0, GAMMA);
	glUniform1f(1, KAPPA);
	glUniform1f(2, REST_DENSITY);
	glUniform1f(3, h2);
	glUniform1f(4, poly6Coe);
	glUniform1ui(5, maxNeighbors);

	// Launch the shader
	static GLint workGroupSize[3] = { -1, -1, -1 };
	glGetProgramiv(shaderID, GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);
	int numGroupsX = nextPowerOfTwo(particles.size()) / workGroupSize[0];
	glDispatchCompute(numGroupsX, 1, 1);

	// Block until complete
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glUseProgram(0);
}
void GLSLSPHDomain::invokeForcesProgram()
{
	// Bind the shader
	GLuint shaderID = computeForcesProgram->getProgramID();
	glUseProgram(shaderID);

	// Binds uniforms
	glUniform1f(0, VISCOSITY);
	glUniform1f(1, h);
	glUniform1f(2, spikyGradCoe);
	glUniform1f(3, polyLapCoe);
	glUniform1ui(4, maxNeighbors);

	// Launch the shader
	static GLint workGroupSize[3] = { -1, -1, -1 };
	glGetProgramiv(shaderID, GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);
	int numGroupsX = nextPowerOfTwo(particles.size()) / workGroupSize[0];
	glDispatchCompute(numGroupsX, 1, 1);

	// Block until complete
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glUseProgram(0);
}
void GLSLSPHDomain::invokeIntegrateProgram(GLfloat dt)
{
	// Bind the shader
	GLuint shaderID = integrateProgram->getProgramID();
	glUseProgram(shaderID);

	// Binds uniforms
	glUniform1f(0, dt);
	glUniform1f(1, FRICTION);
	glUniform1fv(2, 6, bounds);

	// Launch the shader
	static GLint workGroupSize[3] = { -1, -1, -1 };
	glGetProgramiv(shaderID, GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);
	int numGroupsX = nextPowerOfTwo(particles.size()) / workGroupSize[0];
	glDispatchCompute(numGroupsX, 1, 1);

	// Block until complete
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glUseProgram(0);
}