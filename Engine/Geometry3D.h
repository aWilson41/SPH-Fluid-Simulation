#pragma once
#include "MathHelper.h"

// Just a little 3d geometry helper
namespace geom3d
{
	enum ShapeType
	{
		POINT = 0,
		SPHERE = 1,
		RECT = 2,
		POLY = 3,
		RAY = 4
	};

	class Shape
	{
	public:
		Shape();
		Shape(glm::vec3 pos);

	public:
		virtual GLfloat volume();

	public:
		ShapeType type = ShapeType::POINT;
		glm::vec3 pos = glm::vec3(0.0f);
	};

	class Sphere : public Shape
	{
	public:
		Sphere();
		Sphere(GLfloat x, GLfloat y, GLfloat z, GLfloat radius);
		Sphere(glm::vec3 center, GLfloat radius);

	public:
		GLfloat volume() override;

	public:
		GLfloat radius = 0.0f;
	};

	class Rect : public Shape
	{
	public:
		Rect();
		Rect(glm::vec3 center, glm::vec3 size);

	public:
		GLfloat volume() override;
		glm::vec3 size();
		glm::vec3 origin();

	public:
		glm::vec3 extent = glm::vec3(0.0f);
	};

	class Poly : public Shape
	{
	public:
		Poly();
		Poly(glm::vec3* vertices, UINT count);

	public:
		void FromSphere(Sphere sphere, UINT thetaDiv, UINT phiDiv);
		GLfloat volume() override;

	public:
		std::vector<glm::vec3> vertices;
	};

	class Ray : public Shape
	{
	public:
		Ray();
		Ray(glm::vec3 start, glm::vec3 dir);

	public:
		glm::vec3 dir = glm::vec3(1.0f, 0.0f, 0.0f);
	};

	// Returns first intersection
	static bool intersectSphereRay(Sphere sphere, Ray ray)
	{
		glm::vec3 m = ray.pos - sphere.pos;
		GLfloat b = glm::dot(m, ray.dir);
		GLfloat c = glm::dot(m, m) - sphere.radius * sphere.radius;

		// Exit if r’s origin outside s (c > 0) and r pointing away from s (b > 0) 
		if (c > 0.0f && b > 0.0f)
			return false;
		GLfloat discr = b * b - c;

		// A negative discriminant corresponds to ray missing sphere 
		if (discr < 0.0f)
			return false;

		// Ray now found to intersect sphere, compute smallest t value of intersection
		GLfloat t = -b - std::sqrt(discr);

		// If t is negative, ray started inside sphere so clamp t to zero 
		if (t < 0.0f)
			t = 0.0f;
		glm::vec3 q = ray.pos + t * ray.dir;

		return true;
	}
}