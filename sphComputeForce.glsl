#version 460
struct Particle
{
	vec3 position;
		float pad1;
	vec3 velocity;
		float pad2;
	vec3 acceleration;
		float pad3;
	float mass;
	float density;
	float pressure;
		float pad4;
};
// Particle buffer
layout(std430, binding = 0) buffer particleBuffer
{
	Particle data[];
};
// Neighbor buffer
layout(std430, binding = 1) buffer neighborBuffer
{
	uint neighborIds[];
};

// Simulation Parameters
layout(location = 0) uniform float VISCOSITY;
layout(location = 1) uniform float H;
layout(location = 2) uniform float SPIKYGRAD_COE;
layout(location = 3) uniform float POLYLAP_COE;
layout(location = 4) uniform uint MAX_NEIGHBORS;

vec3 g = vec3(0.0f, -9.8f, 0.0f);

// Gradient of Spiky Kernel
vec3 gradKernel(vec3 x)
{
	float r = length(x);
	if (r > H || r < 0.0f)
		return vec3(0.0f);

	float l = H - r;
	return SPIKYGRAD_COE * (x / r) * l * l;
}
// Laplacian of Viscosity Kernel
float laplaceKernel(vec3 x)
{
	float r = length(x);
	if (r > H || r < 0.0f)
		return 0.0f;
	return POLYLAP_COE * (H - r);
}

layout (local_size_x = 64, local_size_y = 1) in;
void main(void)
{
	uint id = gl_GlobalInvocationID.x;
	uint startNeighborIndex = id * MAX_NEIGHBORS + 1;
	uint numNeighbors = neighborIds[startNeighborIndex - 1];

	vec3 fPressure = vec3(0.0f);
	vec3 fViscosity = vec3(0.0f);
	vec3 pos = data[id].position;
	vec3 v1 = data[id].velocity;
	float m1 = data[id].mass;
	float invDensity1 = 1.0f / data[id].density;
	float a = data[id].pressure * invDensity1 * invDensity1;
	for (uint i = startNeighborIndex; i < startNeighborIndex + numNeighbors; i++)
	{
		uint index = neighborIds[i];

		float invDensity2 = 1.0f / data[index].density;
		float m2 = data[index].mass;
		vec3 dist = pos - data[index].position;
		fPressure -= m2 * (a + data[index].pressure * invDensity2 * invDensity2) * gradKernel(dist);
		fViscosity += m2 * (data[index].velocity - v1) * invDensity2 * laplaceKernel(dist);
	}

	data[id].acceleration = (fPressure * m1 + VISCOSITY * fViscosity /* + fSurface*/) * invDensity1 + g;
}