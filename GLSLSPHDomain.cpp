#include "GLSLSPHDomain.h"
#include "Constants.h"
#include <ShaderProgram.h>
#include <Shaders.h>
#include <chrono>

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

GLSLSPHDomain::GLSLSPHDomain()
{
	// Load the compute shader
	computePressureProgram = Shaders::loadComputeShader("main", "Shaders/sphComputePressure.glsl");
	computeForcesProgram = Shaders::loadComputeShader("main", "Shaders/sphComputeForce.glsl");
	integrateProgram = Shaders::loadComputeShader("main", "Shaders/sphIntegrate.glsl");
	binProgram = Shaders::loadComputeShader("main", "Shaders/sphBin.glsl");
}

GLSLSPHDomain::~GLSLSPHDomain()
{
	delete[] gpuParticles;
	glDeleteBuffers(1, &particleBufferID);
	glDeleteBuffers(1, &neighborBufferID);
	glDeleteBuffers(1, &binBufferID);
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

	glm::vec3 dim = bufferSize / h;
	gridWidth = static_cast<int>(dim.x);
	gridHeight = static_cast<int>(dim.y);
	gridDepth = static_cast<int>(dim.z);
	numVoxels = gridWidth * gridHeight * gridDepth;

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

	maxNeighbors = static_cast<GLuint>(particles.size()) / 100;
	bins.resize(maxNeighbors * numVoxels);
	std::fill_n(bins.data(), bins.size(), 0);

	// Put them on the gpu
	glGenBuffers(1, &particleBufferID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBufferID);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUParticle) * particles.size(), gpuParticles, GL_STATIC_DRAW);
	glNamedBufferStorage(particleBufferID, sizeof(GPUParticle) * particles.size(), gpuParticles, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleBufferID); // Point this buffer to binding point 0 in shader

	glGenBuffers(1, &neighborBufferID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, neighborBufferID);
	glNamedBufferStorage(neighborBufferID, sizeof(GLuint) * maxNeighbors * particles.size(), NULL, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, neighborBufferID); // Point this buffer to binding point 1 in shader

	glGenBuffers(1, &binBufferID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, binBufferID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * bins.size(), bins.data(), GL_DYNAMIC_DRAW);
	//glNamedBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * bins.size(), bins.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, binBufferID); // Point this buffer to binding point 2 in shader

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void GLSLSPHDomain::update(GLfloat dt)
{
	// Bin the particles
	std::fill_n(bins.data(), bins.size(), 0);
	for (UINT i = 0; i < particles.size(); i++)
	{
		SPHParticle* p = &particles[i];
		p->gridX = MathHelp::clamp(static_cast<int>(gridWidth * (p->pos->x - bufferBounds[0]) / bufferSize.x), 0, gridWidth - 1);
		p->gridY = MathHelp::clamp(static_cast<int>(gridHeight * (p->pos->y - bufferBounds[2]) / bufferSize.y), 0, gridHeight - 1);
		p->gridZ = MathHelp::clamp(static_cast<int>(gridDepth * (p->pos->z - bufferBounds[4]) / bufferSize.z), 0, gridDepth - 1);
		int gridIndex = calcIndex(p->gridX, p->gridY, p->gridZ, gridWidth, gridHeight); // Index of voxel in array
		int sizeIndex = gridIndex * maxNeighbors;
		GLuint& particleCount = bins[sizeIndex];
		bins[sizeIndex + particleCount + 1] = i;
		particleCount++;
		/*if (particleCount > maxParticlesPerCell)
			printf("ERRORORR!\n");*/
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, binBufferID);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint) * bins.size(), bins.data());
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	// Bind the buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBufferID);

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

void GLSLSPHDomain::invokeBinProgram()
{
	// Bind the shader
	GLuint shaderID = computePressureProgram->getProgramID();
	glUseProgram(shaderID);

	// Binds uniforms
	glUniform1ui(0, maxNeighbors);
	int gridDim[3] = { gridWidth, gridHeight, gridDepth };
	glUniform3iv(1, 1, gridDim);
	glm::vec3 gridOrigin = glm::vec3(bufferBounds[0], bufferBounds[2], bufferBounds[4]);
	glUniform3fv(2, 1, &gridOrigin[0]);
	glUniform3fv(3, 1, &bufferSize[0]);

	// Launch the shader
	static GLint workGroupSize[3] = { -1, -1, -1 };
	glGetProgramiv(shaderID, GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);
	int numGroupsX = nextPowerOfTwo(static_cast<int>(particles.size())) / workGroupSize[0];
	glDispatchCompute(numGroupsX, 1, 1);

	// Block until complete
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glUseProgram(0);
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
	int gridDim[3] = { gridWidth, gridHeight, gridDepth };
	glUniform3iv(6, 1, gridDim);
	glm::vec3 gridOrigin = glm::vec3(bufferBounds[0], bufferBounds[2], bufferBounds[4]);
	glUniform3fv(7, 1, &gridOrigin[0]);
	glUniform3fv(8, 1, &bufferSize[0]);

	// Launch the shader
	static GLint workGroupSize[3] = { -1, -1, -1 };
	glGetProgramiv(shaderID, GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);
	int numGroupsX = nextPowerOfTwo(static_cast<int>(particles.size())) / workGroupSize[0];
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
	int numGroupsX = nextPowerOfTwo(static_cast<int>(particles.size())) / workGroupSize[0];
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
	int numGroupsX = nextPowerOfTwo(static_cast<int>(particles.size())) / workGroupSize[0];
	glDispatchCompute(numGroupsX, 1, 1);

	// Block until complete
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glUseProgram(0);
}