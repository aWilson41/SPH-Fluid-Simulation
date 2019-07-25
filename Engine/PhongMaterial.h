#pragma once
#include "MathHelper.h"

class PhongMaterial
{
public:
	PhongMaterial()
	{
		diffuseColor = glm::vec3(0.0f);
		specularColor = glm::vec4(0.0f);
		ambientColor = glm::vec3(0.0f);
	}
	PhongMaterial(glm::vec3 diffuseColor, glm::vec4 specularColor, glm::vec3 ambientColor)
	{
		PhongMaterial::diffuseColor = diffuseColor;
		PhongMaterial::specularColor = specularColor;
		PhongMaterial::ambientColor = ambientColor;
	}
	PhongMaterial(glm::vec3 diffuseColor, GLfloat fraction)
	{
		PhongMaterial::diffuseColor = diffuseColor;
		specularColor = glm::vec4(0.0f);
		ambientColor = diffuseColor * fraction;
	}

public:
	// Sets the ambient color to a fraction of the diffuse
	void setAmbientToDiffuse(GLfloat fraction = 1.0f) { ambientColor = diffuseColor * fraction; }

	void setDiffuse(glm::vec3 diffuseColor) { PhongMaterial::diffuseColor = diffuseColor; }
	void setDiffuse(GLfloat r, GLfloat g, GLfloat b) { setDiffuse(glm::vec3(r, g, b)); }

	void setSpecular(glm::vec4 specularColor) { PhongMaterial::specularColor = specularColor; }

	void setAmbient(glm::vec3 ambientColor) { PhongMaterial::ambientColor = ambientColor; }
	void setAmbient(GLfloat r, GLfloat g, GLfloat b) { setAmbient(glm::vec3(r, g, b)); }

	glm::vec3 getDiffuse() { return diffuseColor; }
	glm::vec4 getSpecular() { return specularColor; }
	glm::vec3 getAmbient() { return ambientColor; }

protected:
	glm::vec3 diffuseColor = glm::vec3(0.0f);
	glm::vec4 specularColor = glm::vec4(0.0f);
	glm::vec3 ambientColor = glm::vec3(0.0f);
};