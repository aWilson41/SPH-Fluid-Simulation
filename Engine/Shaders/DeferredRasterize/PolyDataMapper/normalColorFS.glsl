#version 460
uniform vec3 lightDir;

smooth in vec3 normal;
smooth in vec3 color;

layout (location = 0) out vec3 gPos;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gColor;

void main()
{
	gPos = vec3(0.0f, 0.0f, 0.0f);
	gNormal = normalize(normal);
	gColor = color;
}