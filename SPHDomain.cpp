#include "SPHDomain.h"
#include "Constants.h"

// Poly6 Kernel
static GLfloat kernel(glm::vec3 x)
{
	GLfloat r2 = glm::dot(x, x);
	if (r2 > h2 || r2 < 0.0f)
		return 0.0f;

	GLfloat l = h2 - r2;
	return poly6Coe * l * l * l;
}
// Gradient of Spiky Kernel
static glm::vec3 gradKernel(glm::vec3 x)
{
	GLfloat r = glm::length(x);
	if (r > h || r < 0.0f)
		return glm::vec3(0.0f, 0.0f, 0.0f);

	GLfloat l = h - r;
	return spikyGradCoe * (x / r) * l * l;
}
// Laplacian of Viscosity Kernel
static GLfloat laplaceKernel(glm::vec3 x)
{
	GLfloat r = glm::length(x);
	if (r > h || r < 0.0f)
		return 0.0f;
	return polyLapCoe * (h - r);
}

static int calcIndex(int x, int y, int z, int width, int height) { return x + width * (y + height * z); }

void SPHDomain::initParticles(std::vector<Particle> particles, glm::vec3 origin, glm::vec3 size, GLfloat bufferRatio)
{
	SPHDomain::particles = particles;
	SPHDomain::origin = origin;
	SPHDomain::size = size;
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
	gridWidth = spacing.x;
	gridHeight = spacing.y;
	gridDepth = spacing.z;
}

// Calculate, density, pressures, and save the neighbors
void SPHDomain::calcDensity()
{
	// Bin the particles into local areas
	std::vector<std::vector<Particle*>> bins(gridWidth * gridHeight * gridDepth);
	for (UINT i = 0; i < particles.size(); i++)
	{
		Particle* p = &particles[i];
		p->gridX = MathHelp::clamp(static_cast<int>(gridWidth * (p->pos->x - bufferBounds[0]) / bufferSize.x), 0, gridWidth - 1);
		p->gridY = MathHelp::clamp(static_cast<int>(gridHeight * (p->pos->y - bufferBounds[2]) / bufferSize.y), 0, gridHeight - 1);
		p->gridZ = MathHelp::clamp(static_cast<int>(gridDepth * (p->pos->z - bufferBounds[4]) / bufferSize.z), 0, gridDepth - 1);
		int binIndex = calcIndex(p->gridX, p->gridY, p->gridZ, gridWidth, gridHeight);
		bins[binIndex].push_back(p);
	}

	// Calculate the density and pressure between particles using the local areas
	for (UINT i = 0; i < particles.size(); i++)
	{
		Particle* p1 = &particles[i];
		GLfloat densitySum = 0.0f;
		p1->neighbors.clear();

		int bounds[6] = {
			MathHelp::clamp(p1->gridX - 1, 0, gridWidth), MathHelp::clamp(p1->gridX + 2, 0, gridWidth),
			MathHelp::clamp(p1->gridY - 1, 0, gridHeight), MathHelp::clamp(p1->gridY + 2, 0, gridHeight),
			MathHelp::clamp(p1->gridZ - 1, 0, gridDepth), MathHelp::clamp(p1->gridZ + 2, 0, gridDepth) };
		for (int z = bounds[4]; z < bounds[5]; z++)
		{
			for (int y = bounds[2]; y < bounds[3]; y++)
			{
				for (int x = bounds[0]; x < bounds[1]; x++)
				{
					int binIndex = calcIndex(x, y, z, gridWidth, gridHeight);
					for (int j = 0; j < bins[binIndex].size(); j++)
					{
						Particle* p2 = bins[binIndex][j];
						glm::vec3 dist = p1->getPos() - p2->getPos();
						// IE: If (dist between centers of spheres < r1 + r2). But for our spheres r1=r2 so just use diameter
						if (glm::dot(dist, dist) <= h2)
						{
							if (p1 != p2)
								p1->neighbors.push_back(p2);
							densitySum += p2->mass * kernel(dist);
						}
					}
				}
			}
		}

		p1->density = densitySum;
		// Pressure = 0 when density = rest density
		//p1.pressure = STIFFNESS * (p1.density - REST_DENSITY);
		p1->pressure = KAPPA * REST_DENSITY / GAMMA * (std::pow(p1->density / REST_DENSITY, GAMMA) - 1.0f); // Taits formulation
	}

	//for (UINT i = 0; i < particles.size(); i++)
	//{
	//	Particle& p1 = particles[i];
	//	GLfloat densitySum = 0.0f;
	//	p1.neighbors.clear();
	//	for (UINT j = 0; j < particles.size(); j++)
	//	{
	//		Particle* p2 = &particles[j];
	//		glm::vec3 dist = p1.getPos() - p2->getPos();
	//		// IE: If (dist between centers of spheres < r1 + r2). But for our spheres r1=r2 so just use diameter
	//		if (glm::dot(dist, dist) <= h2)
	//		{
	//			if (i != j)
	//				p1.neighbors.push_back(p2);
	//			densitySum += p2->mass * kernel(dist);
	//		}
	//	}
	//	p1.density = densitySum;
	//	// Pressure = 0 when density = rest density
	//	//p1.pressure = STIFFNESS * (p1.density - REST_DENSITY);
	//	p1.pressure = KAPPA * REST_DENSITY / GAMMA * (std::pow(p1.density / REST_DENSITY, GAMMA) - 1.0f); // Taits formulation
	//}
}

void SPHDomain::calcForces()
{ 
	glm::vec3 g = glm::vec3(0.0f, -9.8f, 0.0f);
	for (UINT i = 0; i < particles.size(); i++)
	{
		Particle& p1 = particles[i];
		glm::vec3 fPressure = glm::vec3(0.0f);
		glm::vec3 fViscosity = glm::vec3(0.0f);

		for (UINT j = 0; j < p1.neighbors.size(); j++)
		{
			Particle* p2 = p1.neighbors[j];
			glm::vec3 dist = p1.getPos() - p2->getPos();

			// Pressure force density
			//fPressure -= p2->mass * (p2->pressure + p1.pressure) / (2.0f * p2->density) * gradKernel(dist);
			fPressure -= p2->mass * p1.mass * (p1.pressure / (p1.density * p1.density) + p2->pressure / (p2->density * p2->density)) * gradKernel(dist);

			// Viscosity force density
			fViscosity += p2->mass * (p2->velocity - p1.velocity) / p2->density * laplaceKernel(dist);
		}

		p1.accel = (fPressure + VISCOSITY * fViscosity /* + fSurface*/) / p1.density + g;
	}
}

void SPHDomain::collision(glm::vec3 pos, glm::vec3& v)
{
	// Collision
	glm::vec3 normal = glm::vec3(0.0f);
	bool collision = false;
	glm::vec3 vt = glm::vec3(0.0f);
	GLfloat vn = 0.0f;

	GLfloat bounds[6] = { origin.x, origin.x + size.x,
		origin.y, origin.y + size.y,
		origin.z, origin.z + size.z };
	for (UINT i = 0; i < 3; i++)
	{
		collision = false;
		if (i == 0)
		{
			if (pos.x <= bounds[0])
			{
				collision = true;
				normal = glm::vec3(1.0f, 0.0f, 0.0f);
			}
			else if (pos.x >= bounds[1])
			{
				collision = true;
				normal = glm::vec3(-1.0f, 0.0f, 0.0f);
			}
		}
		else if (i == 1)
		{
			if (pos.y <= bounds[2])
			{
				collision = true;
				normal = glm::vec3(0.0f, 1.0f, 0.0f);
			}
			else if (pos.y >= bounds[3])
			{
				collision = true;
				normal = glm::vec3(0.0f, -1.0f, 0.0f);
			}
		}
		else if (i == 2)
		{
			if (pos.z <= bounds[4])
			{
				collision = true;
				normal = glm::vec3(0.0f, 0.0f, 1.0f);
			}
			else if (pos.z >= bounds[5])
			{
				collision = true;
				normal = glm::vec3(0.0f, 0.0f, -1.0f);
			}
		}

		if (collision)
		{
			// If separating, do nothing
			vn = glm::dot(v, normal);
			if (vn >= 0)
				continue;

			// Get tangent velocity by removing velocity in normal dir
			vt = v - vn * normal;
			// Until vt surpasses this value don't let it move (static friction)
			if (glm::length(vt) <= -FRICTION * vn)
			{
				v = glm::vec3(0.0f);
				return;
			}
			// Apply dynamic friction
			v = vt + FRICTION * vn * normalize(vt);
		}
	}
}

void SPHDomain::update(GLfloat dt)
{
	calcDensity();
	calcForces();

	// Integrate the velocity and position and do collision
	for (UINT i = 0; i < particles.size(); i++)
	{
		Particle& p = particles[i];
		particles[i].updateVelocity(dt);
		collision(p.getPos(), p.velocity);
		particles[i].updatePos(dt);
	}
}