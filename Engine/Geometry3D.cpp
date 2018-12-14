#include "Geometry3D.h"

namespace geom3d
{
	// Shape base
	Shape::Shape()
	{
		pos = glm::vec3(0.0f);
		type = POINT;
	}
	Shape::Shape(glm::vec3 pos)
	{
		Shape::pos = pos;
		type = POINT;
	}
	GLfloat Shape::volume() { return 0.0f; }

	// Sphere
	Sphere::Sphere()
	{
		radius = 0.0f;
		type = SPHERE;
	}
	Sphere::Sphere(GLfloat x, GLfloat y, GLfloat z, GLfloat radius)
	{
		pos = glm::vec3(x, y, z);
		Sphere::radius = radius;
		type = SPHERE;
	}
	Sphere::Sphere(glm::vec3 center, GLfloat radius)
	{
		pos = center;
		Sphere::radius = radius;
		type = SPHERE;
	}
	GLfloat Sphere::volume() { return 4.0f * PI * radius * radius * radius / 3.0f; }

	// Rect
	Rect::Rect()
	{
		extent = glm::vec3(0.0f);
		type = RECT;
	}
	Rect::Rect(glm::vec3 center, glm::vec3 size)
	{
		pos = center;
		extent = size * 0.5f;
		type = RECT;
	}
	GLfloat Rect::volume() { return extent.x * extent.y * extent.z * 8.0f; }
	glm::vec3 Rect::size() { return extent * 2.0f; }
	glm::vec3 Rect::origin() { return pos - extent; }

	// Polygon
	Poly::Poly() { type = ShapeType::POLY; }
	Poly::Poly(glm::vec3* vertices, UINT count)
	{
		// Copy the vertices
		for (UINT i = 0; i < count; i++)
		{
			Poly::vertices[i] = vertices[i];
		}
		type = POLY;
		pos = MathHelp::calcCentroid(vertices, count);
	}
	void Poly::FromSphere(Sphere sphere, UINT thetaDiv, UINT phiDiv)
	{

	}
	GLfloat Poly::volume() { return MathHelp::polygonVolume(this); }
}