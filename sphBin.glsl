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
struct BSPNode
{
	float splitplane;
	uint dim; // Dimension to split in
	uint id; // Location in array
	// Children location in array
	uint leftId;
	uint rightId;
};

// Particle buffer
layout(std430, binding = 0) buffer particleBuffer
{
	Particle data[];
};
// BSP buffer, root at first location
layout(std430, binding = 2) buffer nodeBuffer
{
	BSPNode nodes[];
};

layout(location = 0) uniform uint MAX_NEIGHBORS;
// Grid dimensions
layout(location = 1) uniform ivec3 GRID_DIM;
// Grid physical/world origin
layout(location = 2) uniform vec3 GRID_ORIGIN;
// Grid physical size
layout(location = 3) uniform vec3 GRID_SIZE;

int calcIndex(int x, int y, int z, int width, int height) { return x + width * (y + height * z); }

layout (local_size_x = 1, local_size_y = 1) in;
void main(void)
{
	uint id = gl_GlobalInvocationID.x;

	vec3 pos = data[id].position;
	// Compute particles pos in grid
	vec3 gridPosC = GRID_DIM * (pos - GRID_ORIGIN) / GRID_SIZE;
	ivec3 gridPos = ivec3(
		clamp(gridPosC.x, 0, GRID_DIM.x - 1),
		clamp(gridPosC.y, 0, GRID_DIM.y - 1),
		clamp(gridPosC.z, 0, GRID_DIM.z - 1));
	uint gridIndex = calcIndex(gridPos.x, gridPos.y, gridPos.z, GRID_DIM.x, GRID_DIM.y);
	uint sizeIndex = gridIndex * MAX_NEIGHBORS;
	uint particleCount = binIds[sizeIndex];
	//bins[sizeIndex + particleCount + 1] = i;
	binIds[sizeIndex]++; // <- sequential/wont work
}