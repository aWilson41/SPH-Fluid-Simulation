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
	
//	int gridX = clamp(gridWidth * (pos.x - bufferBounds[0]) / bufferSize.x), 0, gridWidth - 1);
//	int gridY = clamp(gridHeight * (pos.y - bufferBounds[2]) / bufferSize.y), 0, gridHeight - 1);
//	int gridZ = clamp(gridDepth * (pos.z - bufferBounds[4]) / bufferSize.z), 0, gridDepth - 1);
//
//	// For all the neighboring grid cells
//	int bounds[6] = {
//			clamp(p1->gridX - 1, 0, gridWidth), clamp(p1->gridX + 2, 0, gridWidth),
//			clamp(p1->gridY - 1, 0, gridHeight), clamp(p1->gridY + 2, 0, gridHeight),
//			clamp(p1->gridZ - 1, 0, gridDepth), clamp(p1->gridZ + 2, 0, gridDepth) };
//	for (int z = bounds[4]; z < bounds[5]; z++)
//	{
//		for (int y = bounds[2]; y < bounds[3]; y++)
//		{
//			for (int x = bounds[0]; x < bounds[1]; x++)
//			{
//				int binIndex = calcIndex(x, y, z, gridWidth, gridHeight);
//				for (UINT j = 0; j < bins[binIndex].size(); j++)
//				{
//			}
//		}
//	}

	data[id].acceleration = (fPressure * m1 + VISCOSITY * fViscosity /* + fSurface*/) * invDensity1 + g;
}