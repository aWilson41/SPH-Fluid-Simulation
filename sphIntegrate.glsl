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

// Simulation Parameters
layout(location = 0) uniform float DT;
layout(location = 1) uniform float FRICTION;
layout(location = 2) uniform float BOUNDS[6];

vec3 collision(vec3 pos, vec3 v)
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
				return v;
			}
			// Apply dynamic friction
			v = vt + FRICTION * vn * normalize(vt);
		}
	}
	return v;
}

layout (local_size_x = 1, local_size_y = 1) in;
void main(void)
{
	uint id = gl_GlobalInvocationID.x;

	vec3 newVelocity = collision(data[id].position, data[id].velocity + data[id].acceleration * DT);
	data[id].velocity = newVelocity;
	data[id].position += newVelocity * DT;
}