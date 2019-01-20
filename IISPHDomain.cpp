#include "IISPHDomain.h"
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

void IISPHDomain::initParticles(std::vector<SPHParticle> particles, glm::vec3 origin, glm::vec3 size, GLfloat bufferRatio)
{
	IISPHDomain::particles.resize(particles.size());
	for (UINT i = 0; i < particles.size(); i++)
	{
		IISPHDomain::particles[i] = IISPHParticle(particles[i]);
	}
	IISPHDomain::origin = origin;
	IISPHDomain::size = size;
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
}

void IISPHDomain::initParticles(std::vector<IISPHParticle> particles, glm::vec3 origin, glm::vec3 size, GLfloat bufferRatio)
{
	IISPHDomain::particles = particles;
	IISPHDomain::origin = origin;
	IISPHDomain::size = size;
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
}

// Calculate, density, pressures, and save the neighbors
void IISPHDomain::calcDensity()
{
	// Bin the particles into local areas
	std::vector<std::vector<Particle*>> bins(gridWidth * gridHeight * gridDepth);
	for (UINT i = 0; i < particles.size(); i++)
	{
		IISPHParticle* p = &particles[i];
		p->gridX = MathHelp::clamp(static_cast<int>(gridWidth * (p->pos->x - bufferBounds[0]) / bufferSize.x), 0, gridWidth - 1);
		p->gridY = MathHelp::clamp(static_cast<int>(gridHeight * (p->pos->y - bufferBounds[2]) / bufferSize.y), 0, gridHeight - 1);
		p->gridZ = MathHelp::clamp(static_cast<int>(gridDepth * (p->pos->z - bufferBounds[4]) / bufferSize.z), 0, gridDepth - 1);
		int binIndex = calcIndex(p->gridX, p->gridY, p->gridZ, gridWidth, gridHeight);
		bins[binIndex].push_back(p);
	}

	// Calculate the density and pressure between particles using the local areas
	for (UINT i = 0; i < particles.size(); i++)
	{
		IISPHParticle* p1 = &particles[i];
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
					for (UINT j = 0; j < bins[binIndex].size(); j++)
					{
						IISPHParticle* p2 = static_cast<IISPHParticle*>(bins[binIndex][j]);
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
	}
}

void IISPHDomain::calcNonPressureForces()
{
	// Compute predicted velocity from non pressure forces
	glm::vec3 g = glm::vec3(0.0f, -9.8f, 0.0f);
	for (UINT i = 0; i < particles.size(); i++)
	{
		IISPHParticle& p1 = particles[i];
		glm::vec3 fPressure = glm::vec3(0.0f);
		glm::vec3 fViscosity = glm::vec3(0.0f);
		p1.dii = glm::vec3(0.0f);

		for (UINT j = 0; j < p1.neighbors.size(); j++)
		{
			IISPHParticle* p2 = static_cast<IISPHParticle*>(p1.neighbors[j]);
			glm::vec3 dist = p1.getPos() - p2->getPos();

			// Viscosity force density
			fViscosity += p2->mass * (p2->velocity - p1.velocity) / p2->density * laplaceKernel(dist);

			p1.dii -= p2->mass / (p1.density * p1.density) * gradKernel(dist);
		}

		// Compute velocity from non-pressure forces, we will then optimize pressure forces
		p1.vAdv = p1.velocity + (VISCOSITY * fViscosity / p1.density + g) * TIMESTEP;
		p1.dii *= TIMESTEP * TIMESTEP;
	}
	// Compute predicted density
	for (UINT i = 0; i < particles.size(); i++)
	{
		IISPHParticle& p1 = particles[i];
		p1.aii = 0.0f;

		GLfloat dp = 0.0f; // Change in pressure
		for (UINT j = 0; j < p1.neighbors.size(); j++)
		{
			IISPHParticle* p2 = static_cast<IISPHParticle*>(p1.neighbors[j]);
			glm::vec3 dist = p1.getPos() - p2->getPos();

			glm::vec3 gradWeight = gradKernel(dist);
			dp += p2->mass * glm::dot((p2->vAdv - p1.vAdv), gradWeight);

			glm::vec3 dji = TIMESTEP * TIMESTEP * p1.mass / (p1.density * p1.density) * gradWeight;
			p1.aii += p2->mass * glm::dot(p1.dii - dji, gradWeight); // Eq12
		}
		p1.projectedDensity = p1.density + dp * TIMESTEP;

		// Set the initial guess
		p1.pl = particles[i].pressure * 0.5f;
	}
}

void IISPHDomain::jacobiPressureSolve()
{
	GLfloat omega = 0.5f;
	//GLfloat tempErrorThreshold = 0.1f;
	GLfloat dt2 = TIMESTEP * TIMESTEP;

	// Finally do the relaxed jacobi iterations
	UINT l = 0;
	while (l < 2)
	{
		for (UINT i = 0; i < particles.size(); i++)
		{
			IISPHParticle& p1 = particles[i];
			p1.dij_pj = glm::vec3(0.0f);

			for (UINT j = 0; j < p1.neighbors.size(); j++)
			{
				IISPHParticle* p2 = static_cast<IISPHParticle*>(p1.neighbors[j]);
				glm::vec3 dist = p1.getPos() - p2->getPos();

				glm::vec3 dij = dt2 * p2->mass / (p2->density * p2->density) * gradKernel(dist);
				p1.dij_pj -= dij * p2->pl;
			}
		}

		for (UINT i = 0; i < particles.size(); i++)
		{
			IISPHParticle& p1 = particles[i];

			GLfloat newPressure = 0.0f;
			for (UINT j = 0; j < p1.neighbors.size(); j++)
			{
				IISPHParticle* p2 = static_cast<IISPHParticle*>(p1.neighbors[j]);
				glm::vec3 dist = p1.getPos() - p2->getPos();

				glm::vec3 gradWeight = gradKernel(dist);
				glm::vec3 dji = dt2 * p1.mass / (p1.density * p1.density) * gradWeight;
				glm::vec3 dji_pi = dji * p1.pl;
				newPressure += p2->mass * glm::dot(p1.dij_pj - p2->dii * p2->pl - (p2->dij_pj - dji_pi), gradWeight);
			}

			// Mix it with the old pressure
			GLfloat bi = REST_DENSITY - p1.projectedDensity;
			p1.pl = (1.0f - omega) * p1.pl + (omega / p1.aii) * (bi - newPressure);
			p1.pressure = p1.pl;
			if (p1.pressure < 0.0f)
				p1.pressure = 0.0f;
		}

		l++;
	}
}

void IISPHDomain::calcPressureForce()
{
	// Integrate the velocity and position and do collision
	for (UINT i = 0; i < particles.size(); i++)
	{
		IISPHParticle& p1 = particles[i];

		// Compute the pressure force from the newly computed pressure
		glm::vec3 fPressure = glm::vec3(0.0f);
		for (UINT j = 0; j < p1.neighbors.size(); j++)
		{
			SPHParticle* p2 = p1.neighbors[j];
			glm::vec3 dist = p1.getPos() - p2->getPos();
			fPressure -= p2->mass * p1.mass * (p1.pressure / (p1.density * p1.density) + p2->pressure / (p2->density * p2->density)) * gradKernel(dist);
		}

		p1.accel = (p1.vAdv - p1.velocity) / TIMESTEP + fPressure / p1.density;
	}
}

void IISPHDomain::update(GLfloat dt)
{
	calcDensity();
	calcNonPressureForces();
	jacobiPressureSolve();
	calcPressureForce();

	// Integrate the velocity and position and do collision
	for (UINT i = 0; i < particles.size(); i++)
	{
		IISPHParticle& p = particles[i];
		p.updateVelocity(dt);
		collision(p.getPos(), p.velocity);
		p.updatePos(dt);
	}
}

void IISPHDomain::collision(glm::vec3 pos, glm::vec3& v)
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