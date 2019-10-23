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
layout(location = 0) uniform float GAMMA;
layout(location = 1) uniform float KAPPA;
layout(location = 2) uniform float REST_DENSITY;
layout(location = 3) uniform float H2;
layout(location = 4) uniform float POLY6_COE;
layout(location = 5) uniform uint MAX_NEIGHBORS;

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
	vec3 p1 = data[id].position;
	for (uint i = 0; i < data.length(); i++)
	{
		vec3 dist = p1 - data[i].position;
		if (dot(dist, dist) <= H2)
		{
			densitySum += data[i].mass * kernel(dist);
			if (i != id)
			{
				neighborIds[startNeighborIndex + neighborCounter] = i;
				neighborCounter++;
			}
		}
	}
	neighborIds[startNeighborIndex - 1] = neighborCounter;

	data[id].density = densitySum;
	float p = KAPPA * REST_DENSITY / GAMMA * (pow(densitySum / REST_DENSITY, GAMMA) - 1.0f);
	data[id].pressure = max(0.0f, p);
}