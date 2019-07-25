#version 460
struct Material
{
	vec3 diffuseColor;
	vec3 ambientColor;
};

uniform Material mat;
uniform vec3 lightDir;

smooth in vec3 normal;

layout (location = 0) out vec3 gPos;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gColor;

void main()
{
	gColor = mat.diffuseColor
	vec3 color = clamp(dot(lightDir, normal) * mat.diffuseColor + mat.ambientColor, 0.0f, 1.0f);
	fColor = vec4(color, 1.0f);
}