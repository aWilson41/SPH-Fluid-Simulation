#version 460
uniform mat4 mvp_matrix;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inOffset;

smooth out vec3 pos;
smooth out vec3 normal;

void main()
{
	// Calculate vertex position in screen space
	gl_Position = mvp_matrix * vec4(inPos + inOffset, 1.0);
	pos = vec3(gl_Position);
	normal = inNormal;
}