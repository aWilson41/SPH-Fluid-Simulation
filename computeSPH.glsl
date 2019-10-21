#version 460
struct Particle
{
	vec3 position;
	vec3 velocity;
	vec3 acceleration;
	float mass;
	float density;
	float pressure;
};

// Particle buffer
layout(std430, binding = 0) buffer particleBuffer
{
	Particle data[];
};

// Simulation Parameters
layout(location = 0) uniform float DT;
layout(location = 1) uniform float GAMMA;
layout(location = 2) uniform float KAPPA;
layout(location = 3) uniform float REST_DENSITY;
layout(location = 4) uniform float FRICTION;
layout(location = 5) uniform float VISCOSITY;

layout(location = 6) uniform float H;
layout(location = 7) uniform float H2;
layout(location = 8) uniform float POLY6_COE;
layout(location = 9) uniform float SPIKYGRAD_COE;
layout(location = 10) uniform float POLYLAP_COE;
layout(location = 11) uniform float BOUNDS[6];

// For "megakernel"
layout(location = 17) uniform uint TASKID;

// Poly6 Kernel
float kernel(vec3 x)
{
	float r2 = dot(x, x);
	if (r2 > H2 || r2 < 0.0f)
		return 0.0f;

	float l = H2 - r2;
	return POLY6_COE * l * l * l;
}
// Gradient of Spiky Kernel
vec3 gradKernel(vec3 x)
{
	float r = length(x);
	if (r > H || r < 0.0f)
		return vec3(0.0f, 0.0f, 0.0f);

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

// Computes the density and pressure of a particle
void computeDensity(uint id)
{
	float densitySum = 0.0f;

	Particle p1 = data[id];
	for (uint i = 0; i < data.length(); i++)
	{
		if (i == id)
			break;
		Particle p2 = data[i];
		vec3 dist = p1.position - p2.position;
		if (dot(dist, dist) <= H2)
			densitySum += p2.mass * kernel(dist);
	}

	data[id].density = densitySum;
	p1.pressure = KAPPA * REST_DENSITY / GAMMA * (pow(p1.density / REST_DENSITY, GAMMA) - 1.0f);
	p1.pressure = max(0.0f, p1.pressure);
}

// Computes the force on a particle
void computeForce(uint id)
{
	vec3 g = vec3(0.0f, -9.8f, 0.0f);

	Particle p1 = data[id];
	vec3 fPressure = vec3(0.0f);
	vec3 fViscosity = vec3(0.0f);
	for (uint i = 0; i < data.length(); i++)
	{
		Particle p2 = data[i];
		vec3 dist = p1.position - p2.position;
		fPressure -= p2.mass * p1.mass * (p1.pressure / (p1.density * p1.density) + p2.pressure / (p2.density * p2.density)) * gradKernel(dist);
		fViscosity += p2.mass * (p2.velocity - p1.velocity) / p2.density * laplaceKernel(dist);
	}

	p1.acceleration = (fPressure + VISCOSITY * fViscosity /* + fSurface*/) / p1.density + g;
}

void collision(vec3 pos, inout vec3 v)
{
	// Collision
	vec3 normal = vec3(0.0f);
	bool collision = false;
	vec3 vt = vec3(0.0f);
	float vn = 0.0f;

	for (uint i = 0; i < 3; i++)
	{
		collision = false;
		if (i == 0)
		{
			if (pos.x <= BOUNDS[0])
			{
				collision = true;
				normal = vec3(1.0f, 0.0f, 0.0f);
			}
			else if (pos.x >= BOUNDS[1])
			{
				collision = true;
				normal = vec3(-1.0f, 0.0f, 0.0f);
			}
		}
		else if (i == 1)
		{
			if (pos.y <= BOUNDS[2])
			{
				collision = true;
				normal = vec3(0.0f, 1.0f, 0.0f);
			}
			else if (pos.y >= BOUNDS[3])
			{
				collision = true;
				normal = vec3(0.0f, -1.0f, 0.0f);
			}
		}
		else if (i == 2)
		{
			if (pos.z <= BOUNDS[4])
			{
				collision = true;
				normal = vec3(0.0f, 0.0f, 1.0f);
			}
			else if (pos.z >= BOUNDS[5])
			{
				collision = true;
				normal = vec3(0.0f, 0.0f, -1.0f);
			}
		}

		if (collision)
		{
			// If separating, do nothing
			vn = dot(v, normal);
			if (vn >= 0)
				continue;

			// Get tangent velocity by removing velocity in normal dir
			vt = v - vn * normal;
			// Until vt surpasses this value don't let it move (static friction)
			if (length(vt) <= -FRICTION * vn)
			{
				v = vec3(0.0f);
				return;
			}
			// Apply dynamic friction
			v = vt + FRICTION * vn * normalize(vt);
		}
	}
}

// Integrates forces and velocity
// Also does collision
void integrate(uint id)
{
	Particle p1 = data[id];
	p1.velocity += p1.acceleration * DT;
	collision(p1.position, p1.velocity);
	p1.position += p1.velocity * DT;
}

layout (local_size_x = 32, local_size_y = 1) in;
void main(void)
{
	uint particleId = gl_GlobalInvocationID.x;
//	switch(TASKID)
//	{
//	case 0:
//		computeDensity(particleId);
//		break;
//	case 1:
//		computeForce(particleId);
//		break;
//	case 2:
//		integrate(particleId);
//		break;
//	default:
//		break;
//	};
	data[particleId].position.x += 0.001f;
}