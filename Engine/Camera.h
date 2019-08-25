#pragma once
#include "MathHelper.h"
#include "Geometry3D.h"

class Camera
{
public:
	Camera() { reset(); }

	// Maps all parameters of Camera
	void initCamera(GLfloat fov, GLfloat aspectRatio, GLfloat nearZ, GLfloat farZ, glm::vec3 eyePos, glm::vec3 focalPt, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f))
	{
		setEyePos(eyePos);
		setFocalPt(focalPt);
		Camera::up = up;
		updateLookAt();

		setPerspective(fov, aspectRatio, nearZ, farZ);
	}

	// Resets the camera to defaults
	virtual void reset() { initCamera(45.0f, 16.0f / 9.0f, 0.0001f, 1000.0f, glm::vec3(1.0f), glm::vec3(0.0f)); }

	void setPerspective(GLfloat fov, GLfloat aspectRatio, GLfloat nearZ, GLfloat farZ)
	{
		Camera::fov = fov;
		Camera::aspectRatio = aspectRatio;
		Camera::nearZ = nearZ;
		Camera::farZ = farZ;
		proj = glm::perspective(fov, aspectRatio, nearZ, farZ);
	}
	void setOrtho(GLfloat left, GLfloat right, GLfloat top, GLfloat bottom, GLfloat nearZ, GLfloat farZ)
	{
		Camera::nearZ = nearZ;
		Camera::farZ = farZ;
		proj = glm::ortho(left, right, top, bottom, nearZ, farZ);
	}
	void setEyePos(glm::vec3 pos) { eyePos = pos; }
	void setEyePos(GLfloat x, GLfloat y, GLfloat z) { setEyePos(glm::vec3(x, y, z)); }

	void setFocalPt(glm::vec3 pt) { focalPt = pt; }
	void setFocalPt(GLfloat x, GLfloat y, GLfloat z) { setFocalPt(glm::vec3(x, y, z)); }
	void updateLookAt() { view = glm::lookAt(eyePos, focalPt, up); }

	geom3d::Ray getEyeRay(glm::vec2 ndc)
	{
		glm::vec4 worldPos = glm::inverse(proj * view) * glm::vec4(ndc, 0.0f, 1.0f);
		return geom3d::Ray(eyePos, glm::normalize(glm::vec3(worldPos) / worldPos.w - eyePos));
	}
	geom3d::Ray getEyeRay(GLfloat x, GLfloat y) { return getEyeRay(glm::vec2(x, y)); }

	glm::vec3 getEyePos() { return eyePos; }
	glm::vec3 getFocalPt() { return focalPt; }
	glm::vec3 getUp() { return up; }

public:
	// Camera view matrix
	glm::mat4 view = glm::mat4(1.0f);
	// Camera projection matrix
	glm::mat4 proj = glm::mat4(1.0f);
	// Eye position
	glm::vec3 eyePos = glm::vec3(1.0f);
	// Focal point
	glm::vec3 focalPt = glm::vec3(0.0f);
	// Up
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	// Camera specs
	GLfloat fov = 45.0f;
	GLfloat aspectRatio = 16.0f / 9.0f;
	GLfloat nearZ = 0.0001f;
	GLfloat farZ = 1000.0f;
};