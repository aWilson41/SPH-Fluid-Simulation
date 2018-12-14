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
		POLY = 3
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
}