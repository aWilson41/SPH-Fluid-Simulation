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
// Bin buffer
layout(std430, binding = 2) buffer binBuffer
{
	uint binIds[];
};

// Simulation Parameters
layout(location = 0) uniform float GAMMA;
layout(location = 1) uniform float KAPPA;
layout(location = 2) uniform float REST_DENSITY;
layout(location = 3) uniform float H2;
layout(location = 4) uniform float POLY6_COE;
layout(location = 5) uniform uint MAX_NEIGHBORS;

// Grid dimensions
layout(location = 6) uniform ivec3 GRID_DIM;
// Grid physical/world origin
layout(location = 7) uniform vec3 GRID_ORIGIN;
// Grid physical size
layout(location = 8) uniform vec3 GRID_SIZE;

// Poly6 Kernel
float kernel(vec3 x)
{
	float r2 = dot(x, x);
	if (r2 > H2 || r2 < 0.0f)
		return 0.0f;

	float l = H2 - r2;
	return POLY6_COE * l * l * l;
}

layout (local_size_x = 32, local_size_y = 1) in;
void main(void)
{
	uint id = gl_GlobalInvocationID.x;
	uint startNeighborIndex = id * MAX_NEIGHBORS + 1; // Starting index of neighbors array
	uint neighborCounter = 0; // Start at 1 as first int is size

	float densitySum = 0.0f;
	vec3 pos = data[id].position;

	// Compute particles pos in grid
	vec3 gridPosC = GRID_DIM * (pos - GRID_ORIGIN) / GRID_SIZE;
	ivec3 gridPos = ivec3(
		clamp(gridPosC.x, 0, GRID_DIM.x - 1),
		clamp(gridPosC.y, 0, GRID_DIM.y - 1),
		clamp(gridPosC.z, 0, GRID_DIM.z - 1));

	// For all the neighboring grid cells
	int bounds[6] = {
			clamp(gridPos.x - 1, 0, GRID_DIM.x), clamp(gridPos.x + 2, 0, GRID_DIM.x),
			clamp(gridPos.y - 1, 0, GRID_DIM.y), clamp(gridPos.y + 2, 0, GRID_DIM.y),
			clamp(gridPos.z - 1, 0, GRID_DIM.z), clamp(gridPos.z + 2, 0, GRID_DIM.z) };
	for (uint z = bounds[4]; z < bounds[5]; z++)
	{
		for (uint y = bounds[2]; y < bounds[3]; y++)
		{
			for (uint x = bounds[0]; x < bounds[1]; x++)
			{
				uint gridIndex = x + GRID_DIM.x * (y + GRID_DIM.y * z);
				// Compute starting index of particles in cell
				uint startBinIndex = gridIndex * MAX_NEIGHBORS + 1;
				uint cellParticleCount = binIds[startBinIndex - 1];

				// For every particle in the cell
				for (uint i = startBinIndex; i < startBinIndex + cellParticleCount; i++)
				{
					uint id2 = binIds[i];
					vec3 dist = pos - data[id2].position;
					if (dot(dist, dist) <= H2)
					{
						densitySum += data[id2].mass * kernel(dist);
						if (id2 != id)
						{
							neighborIds[startNeighborIndex + neighborCounter] = id2;
							neighborCounter++;
						}
					}
				}
			}
		}
	}

	neighborIds[startNeighborIndex - 1] = neighborCounter;

	data[id].density = densitySum;
	//data[id].density = binIds[(gridPos.x + GRID_DIM.x * (gridPos.y + GRID_DIM.y * gridPos.z)) * MAX_NEIGHBORS];
	float p = KAPPA * REST_DENSITY / GAMMA * (pow(densitySum / REST_DENSITY, GAMMA) - 1.0f);
	data[id].pressure = max(0.0f, p);
}