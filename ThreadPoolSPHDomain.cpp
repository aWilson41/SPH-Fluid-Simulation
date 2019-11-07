#include "ThreadPoolSPHDomain.h"
#include "Constants.h"
#include <StdMultiThreaderPool.h>
#include <chrono>
#include <KdTreePointLocator.h>
#include <KdTree.h>

// Poly6 Kernel
static GLfloat kernel(GLfloat r2)
{
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
		return glm::vec3(0.0f);

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

static void threadedCalcDensity(ThreadInfo* threadInfo)
{
	ThreadPoolSPHDomain* sphDomain = static_cast<ThreadPoolSPHDomain*>(threadInfo->UserData);
	sphDomain->calcDensity(threadInfo->ThreadID, threadInfo->NumberOfThreads);
}
static void threadedCalcForces(ThreadInfo* threadInfo)
{
	ThreadPoolSPHDomain* sphDomain = static_cast<ThreadPoolSPHDomain*>(threadInfo->UserData);
	sphDomain->calcForces(threadInfo->ThreadID, threadInfo->NumberOfThreads);
}
static void threadedIntegrate(ThreadInfo* threadInfo)
{
	ThreadPoolSPHDomain* sphDomain = static_cast<ThreadPoolSPHDomain*>(threadInfo->UserData);
	sphDomain->integrate(threadInfo->ThreadID, threadInfo->NumberOfThreads);
}

ThreadPoolSPHDomain::ThreadPoolSPHDomain()
{
	kdtree = new KdTree();
	kdtree->setLeafSize(20);
	threader = new StdMultiThreaderPool();
	threader->start();
}
ThreadPoolSPHDomain::~ThreadPoolSPHDomain()
{
	delete kdtree;
	delete threader;
}

void ThreadPoolSPHDomain::initParticles(std::vector<SPHParticle> particles, glm::vec3 origin, glm::vec3 size, GLfloat bufferRatio)
{
	ThreadPoolSPHDomain::particles = particles;
	ThreadPoolSPHDomain::origin = origin;
	ThreadPoolSPHDomain::size = size;
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
}

// Calculate, density, pressures, and save the neighbors
void ThreadPoolSPHDomain::calcDensity(int threadID, int numThreads)
{
	KdTreePointLocator locator;
	locator.setKdTree(kdtree);

	// Calculate the density and pressure between particles using the local areas
	for (UINT i = threadID; i < particles.size(); i += numThreads)
	{
		SPHParticle* p1 = &particles[i];
		GLfloat densitySum = 0.0f;
		p1->neighbors.clear();

		std::vector<UINT> neighbors = locator.getPoints(p1->getPos(), r);
		for (UINT j = 0; j < neighbors.size(); j++)
		{
			const UINT p2Index = kdtree->getIndices()[neighbors[j]];
			SPHParticle* p2 = &particles[p2Index];
			const glm::vec3 dist = p1->getPos() - p2->getPos();
			const GLfloat r2 = glm::dot(dist, dist);
			if (p1 != p2)
				p1->neighbors.push_back(p2);
			densitySum += p2->mass * kernel(r2);
		}

		p1->density = densitySum;
#ifdef IDEALGAS
		p1->pressure = STIFFNESS * (p1->density - REST_DENSITY);
#else
		// Pressure = 0 when density = rest density
		p1->pressure = KAPPA * REST_DENSITY / GAMMA * (std::pow(p1->density / REST_DENSITY, GAMMA) - 1.0f); // Taits formulation
#endif
		if (p1->pressure < 0.0f)
			p1->pressure = 0.0f;
	}
}

void ThreadPoolSPHDomain::calcForces(int threadID, int numThreads)
{
	glm::vec3 g = glm::vec3(0.0f, -9.8f, 0.0f);
	for (UINT i = threadID; i < particles.size(); i += numThreads)
	{
		SPHParticle& p1 = particles[i];
		glm::vec3 fPressure = glm::vec3(0.0f);
		glm::vec3 fViscosity = glm::vec3(0.0f);
		GLfloat invDensity1 = 1.0f / p1.density;
		GLfloat a = p1.pressure * invDensity1 * invDensity1;
		for (UINT j = 0; j < p1.neighbors.size(); j++)
		{
			SPHParticle* p2 = p1.neighbors[j];
			glm::vec3 dist = p1.getPos() - p2->getPos();
			GLfloat invDensity2 = 1.0f / p2->density;

			// Pressure force density
			fPressure -= p2->mass * (a + p2->pressure * invDensity2 * invDensity2) * gradKernel(dist);

			// Viscosity force density
			fViscosity += p2->mass * (p2->velocity - p1.velocity) * invDensity2 * laplaceKernel(dist);
		}

		p1.accel = (fPressure * p1.mass + VISCOSITY * fViscosity /* + fSurface*/) * invDensity1 + g;
	}
}

void ThreadPoolSPHDomain::integrate(int threadID, int numThreads)
{
	// Integrate the velocity and position and do collision
	for (UINT i = threadID; i < particles.size(); i += numThreads)
	{
		Particle& p = particles[i];
		p.updateVelocity(dt);
		collision(*(p.pos), p.velocity);
		p.updatePos(dt);
	}
}

void ThreadPoolSPHDomain::collision(glm::vec3& pos, glm::vec3& v)
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
				pos.x = bounds[0];
			}
			else if (pos.x >= bounds[1])
			{
				collision = true;
				normal = glm::vec3(-1.0f, 0.0f, 0.0f);
				pos.x = bounds[1];
			}
		}
		else if (i == 1)
		{
			if (pos.y <= bounds[2])
			{
				collision = true;
				normal = glm::vec3(0.0f, 1.0f, 0.0f);
				pos.y = bounds[2];
			}
			else if (pos.y >= bounds[3])
			{
				collision = true;
				normal = glm::vec3(0.0f, -1.0f, 0.0f);
				pos.y = bounds[3];
			}
		}
		else if (i == 2)
		{
			if (pos.z <= bounds[4])
			{
				collision = true;
				normal = glm::vec3(0.0f, 0.0f, 1.0f);
				pos.z = bounds[4];
			}
			else if (pos.z >= bounds[5])
			{
				collision = true;
				normal = glm::vec3(0.0f, 0.0f, -1.0f);
				pos.z = bounds[5];
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

void ThreadPoolSPHDomain::update(GLfloat dt)
{
	ThreadPoolSPHDomain::dt = dt;

	//auto timeStart = std::chrono::steady_clock::now();

	// Bin the particles into local areas
	/*bins = std::vector<std::vector<SPHParticle*>>(gridWidth * gridHeight * gridDepth);
	for (UINT i = 0; i < particles.size(); i++)
	{
		SPHParticle* p = &particles[i];
		p->gridX = MathHelp::clamp(static_cast<int>(gridWidth * (p->pos->x - bufferBounds[0]) / bufferSize.x), 0, gridWidth - 1);
		p->gridY = MathHelp::clamp(static_cast<int>(gridHeight * (p->pos->y - bufferBounds[2]) / bufferSize.y), 0, gridHeight - 1);
		p->gridZ = MathHelp::clamp(static_cast<int>(gridDepth * (p->pos->z - bufferBounds[4]) / bufferSize.z), 0, gridDepth - 1);
		int binIndex = calcIndex(p->gridX, p->gridY, p->gridZ, gridWidth, gridHeight);
		bins[binIndex].push_back(p);
	}*/
	
	kdtree->setAccessor([&](UINT index) { return particles[index].getPos(); }, particles.size());
	kdtree->setLocalOrder(true);
	kdtree->update();

	/*auto timeEnd = std::chrono::steady_clock::now();
	printf("Bin Time: %f\n", std::chrono::duration<double, std::milli>(timeEnd - timeStart).count() / 1000.0);*/

	threader->setMethod(threadedCalcDensity, this);
	threader->execute();
	threader->setMethod(threadedCalcForces, this);
	threader->execute();
	threader->setMethod(threadedIntegrate, this);
	threader->execute();
}